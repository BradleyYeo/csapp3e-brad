#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Counts numeric digits in a null-terminated string.
 * Uses pointer traversal instead of array indexing.
 * Input: text pointer (read-only).
 * Output: count of characters in range ['0'..'9'].
 * Example: "a1b2c" -> 2
 */
static size_t count_digits(const char *text) {
  size_t count = 0;

  for (const char *p = text; *p != '\0'; ++p) {
    if (isdigit((unsigned char)*p)) {
      count++;
    }
  }

  return count;
}

static size_t count_alpha(const char *text) {
  size_t count = 0;
  for (const char *p = text; *p != 0; ++p) {
    if (isalpha((unsigned char)*p)) {
      count++;
    }
  }
  return count;
}

/*
 * Finds the first numeric digit in text.
 * Advances cursor until a digit or null byte is reached.
 * Input: text pointer (read-only).
 * Output: pointer to first digit character, or nullptr if none.
 * Example: "alpha9omega" -> pointer to '9'
 */
static const char *find_first_digit(const char *text) {
  for (const char *p = text; *p != '\0'; ++p) {
    if (isdigit((unsigned char)*p)) {
      return p;
    }
  }

  return nullptr;
}

/*
 * Finds the last numeric digit in text.
 * Traverses backward from null terminator down to text start.
 * Input: text pointer (read-only).
 * Output: pointer to last digit character, or nullptr if none.
 * Example: "a1b2c" -> pointer to '2'
 */
static const char *find_last_digit(const char *text) {
  // Guard against null pointer or empty string.
  if (text == nullptr || *text == '\0') {
    return nullptr;
  }
  const char *p = text;
  // Advance pointer to the null terminator '\0'.
  while (*p != '\0') {
    ++p;
  }
  // Traverse backward while cursor is strictly greater than buffer start.
  while (p > text) {
    // Decrement first, check if character is digit, and return match.
    --p;
    if (isdigit((unsigned char)*p)) {
      return p;
    }
  }

  // Return nullptr when no digit exists in buffer.

  return nullptr;
}

/*
 * Drill: Two-Pointer Lockstep Prefix Matching
 * Implement:
 *   static bool match_prefix(const char *text, const char *prefix)
 *
 * Requirements:
 * - Return true if text starts with prefix, false otherwise.
 * - If prefix is empty (""), return true.
 * - If text or prefix is nullptr, return false.
 * - Pure pointer advancement (*p, ++p); no array indexing ([]), no strlen.
 */

static bool match_prefix(const char *text, const char *prefix) {
  if (text == nullptr || prefix == nullptr) {
    return false;
  } else if (*prefix == '\0') {
    return true;
  }
  const char *s = text;
  while (*s == *prefix && *prefix != '\0') {
    s++;
    prefix++;
  }
  return *prefix == '\0';
}

/*
 * Drill: Substring Search via Cursor Sliding and match_prefix
 * Implement:
 *   static const char *find_substring(const char *text, const char *pattern)
 *
 * Requirements:
 * - Return pointer to first match of pattern in text, or nullptr if none.
 * - If pattern is empty (""), return text.
 * - If text or pattern is nullptr, return nullptr.
 * - Slide cursor through text; call match_prefix(cursor, pattern) at each step.
 */
static const char *find_substring(const char *text, const char *pattern) {
  if (pattern == nullptr || text == nullptr) {
    return nullptr;
  }
  if (*pattern == '\0') {
    return text;
  }
  for (const char *cursor = text; *cursor != '\0'; ++cursor) {
    if (match_prefix(cursor, pattern)) {
      return cursor;
    }
  }
  return nullptr;
}


int main(void) {
  // Drill tests: find_substring
  const char target[] = "catch a catfish";
  const char *sub1 = find_substring(target, "cat");
  assert(sub1 == target);

  const char *sub2 = find_substring(target, "fish");
  assert(sub2 != nullptr && sub2 - target == 11);

  assert(find_substring(target, "dog") == nullptr);
  assert(find_substring(target, "") == target);
  assert(find_substring("", "cat") == nullptr);
  assert(find_substring(nullptr, "cat") == nullptr);
  assert(find_substring(target, nullptr) == nullptr);

  // Test 1: Counting digits
  assert(count_digits("abc") == 0);
  assert(count_digits("12345") == 5);
  assert(count_digits("a1b2c3d") == 3);
  assert(count_digits("") == 0);
  assert(count_alpha("adsc") == 4);
  // Test: find_last_digit boundary conditions and traversal
  assert(find_last_digit(nullptr) == nullptr);
  assert(find_last_digit("") == nullptr);
  assert(find_last_digit("no_digits_here") == nullptr);

  const char start_digit[] = "9hello";
  const char *res_start = find_last_digit(start_digit);
  assert(res_start != nullptr && *res_start == '9');
  assert(res_start - start_digit == 0);

  const char end_digit[] = "hello9";
  const char *res_end = find_last_digit(end_digit);
  assert(res_end != nullptr && *res_end == '9');
  assert(res_end - end_digit == 5);

  const char multi_digits[] = "abc1def2ghi";
  const char *res_multi = find_last_digit(multi_digits);
  assert(res_multi != nullptr && *res_multi == '2');
  assert(res_multi - multi_digits == 7);

  // Test 2: Cursor pointer advancement
  const char sample[] = "hello7world";
  const char *digit_ptr = find_first_digit(sample);

  assert(digit_ptr != nullptr);
  assert(*digit_ptr == '7');
  assert(digit_ptr - sample == 5);

  assert(find_first_digit("no_digits_here") == nullptr);

  // Drill tests: match_prefix
  assert(match_prefix("hello", "hel") == true);
  assert(match_prefix("hello", "hello") == true);
  assert(match_prefix("hello", "") == true);
  assert(match_prefix("hello", "world") == false);
  assert(match_prefix("hi", "hello") == false);
  assert(match_prefix("", "a") == false);
  assert(match_prefix(nullptr, "a") == false);
  assert(match_prefix("a", nullptr) == false);

  printf("[PASS] Exercise 01: Pointer Traversal verified.\n");
  return EXIT_SUCCESS;
}
