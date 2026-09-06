#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Drill 1: Pointer Bounds and Distance Calculation
 *
 * Pattern memory layout:
 * ['[', 'a', 'b', 'c', ']', '\0']
 *   ^    ^              ^
 *   |    start          end
 * pattern (0x10)
 * start = pattern + 1 (0x11)
 * end   = strchr(start, ']') (0x14)
 * len   = end - start = 0x14 - 0x11 = 3 bytes
 *
 * Expected input: pattern starting with '['.
 * Expected output: true if valid closing ']' found, false otherwise.
 * Populates *start_out and *len_out.
 */
static bool get_group_bounds(const char *pattern, const char **start_out, size_t *len_out) {
  if (pattern == nullptr || pattern[0] != '[') {
    return false;
  }

  const char *start = pattern + 1;
  const char *end = strchr(start, ']');

  if (end == nullptr) {
    return false;
  }

  *start_out = start;
  *len_out = (size_t)(end - start);
  return true;
}

/*
 * Drill 2: Buffer Slicing and Explicit Null-Termination
 *
 * Memory invariant:
 * memcpy copies raw bytes without appending '\0'.
 * Reading dst as a string without '\0' causes buffer overrun.
 *
 * Dest buffer layout after memcpy(dest, "abc", 3):
 * ['a', 'b', 'c', '?', '?', ...]
 *                  ^
 *                  dest[len] MUST be explicitly set to '\0'.
 *
 * Expected input: src pointer, byte count len, dest buffer, dest capacity.
 * Expected output: true on successful copy and null-termination, false on overflow.
 */
static bool slice_to_buffer(const char *src, size_t len, char *dest, size_t capacity) {
  if (src == nullptr || dest == nullptr) {
    return false;
  }

  // Prevent buffer overflow: must leave space for terminating null byte
  if (len >= capacity) {
    return false;
  }

  memcpy(dest, src, len);
  dest[len] = '\0';
  return true;
}

/*
 * Drill 3: Character Set Search via strpbrk
 *
 * strpbrk(s, accept) scans s and stops at the FIRST byte matching ANY character in accept.
 *
 * Example:
 * s      = "apple"
 * accept = "xyzpa"
 * strpbrk returns pointer to 'a' at s[0].
 *
 * Expected input: null-terminated text and charset strings.
 * Expected output: pointer to first matching character in text, or nullptr.
 */
static const char *find_any_char(const char *text, const char *charset) {
  if (text == nullptr || charset == nullptr) {
    return nullptr;
  }

  return strpbrk(text, charset);
}

/*
 * Drill 4: Recursive Matcher on String Pointers
 *
 * Demonstrates recursive pointer progression:
 * - Base Case 1: text is exhausted (nullptr or '\0') -> return false.
 * - Base Case 2: strpbrk finds no match -> return false.
 * - Match Case: candidate found -> return true.
 *
 * Expected input: text pointer, null-terminated charset.
 * Expected output: true if any character in charset exists in text.
 */
static bool match_charset_rec(const char *text, const char *charset) {
  if (text == nullptr || *text == '\0') {
    return false;
  }

  const char *candidate = strpbrk(text, charset);
  if (candidate == nullptr) {
    return false;
  }

  // Found a candidate match at *candidate
  return true;
}

/*
 * Drill 5: Full Positive Character Group Matcher
 *
 * Composes bounds extraction, safe stack buffer slicing, and candidate searching.
 *
 * Expected input: input_line to search, pattern like "[abc]".
 * Expected output: true if any char in group matches input_line, false otherwise.
 */
static bool match_pos_bracket_grp(const char *input_line, const char *pattern) {
  const char *start = nullptr;
  size_t len = 0;

  if (!get_group_bounds(pattern, &start, &len)) {
    return false;
  }

  char charset[256];
  if (!slice_to_buffer(start, len, charset, sizeof(charset))) {
    return false;
  }

  return match_charset_rec(input_line, charset);
}

int main(void) {
  // Test 1: Bounds and pointer distance calculation
  const char *start = nullptr;
  size_t len = 0;

  assert(get_group_bounds("[abc]", &start, &len) == true);
  assert(*start == 'a');
  assert(len == 3);

  assert(get_group_bounds("[]", &start, &len) == true);
  assert(len == 0);

  assert(get_group_bounds("[unclosed", &start, &len) == false);
  assert(get_group_bounds("not_a_group", &start, &len) == false);

  // Test 2: Buffer slicing and null-termination
  char buf[8];
  assert(slice_to_buffer("abcdef", 3, buf, sizeof(buf)) == true);
  assert(strcmp(buf, "abc") == 0);
  assert(buf[3] == '\0');

  // Overflow prevention test
  assert(slice_to_buffer("123456789", 9, buf, sizeof(buf)) == false);

  // Test 3: strpbrk set search
  const char *p1 = find_any_char("apple", "xyz");
  assert(p1 == nullptr);

  const char *p2 = find_any_char("apple", "pl");
  assert(p2 != nullptr);
  assert(*p2 == 'p');

  // Test 4: Recursive charset matcher
  assert(match_charset_rec("blueberry", "aeiou") == true);
  assert(match_charset_rec("rhythm", "aeiou") == false);
  assert(match_charset_rec("", "abc") == false);

  // Test 5: End-to-end character group matching
  assert(match_pos_bracket_grp("apple", "[abc]") == true);
  assert(match_pos_bracket_grp("cab", "[abc]") == true);
  assert(match_pos_bracket_grp("dog", "[abc]") == false);
  assert(match_pos_bracket_grp("a1b2c3", "[123]") == true);
  assert(match_pos_bracket_grp("e", "[grape]") == true);
  assert(match_pos_bracket_grp("xyz", "[grape]") == false);

  printf("[PASS] Exercise 05: Character Group Slicing verified.\n");
  return EXIT_SUCCESS;
}
