/*
 * Compile and run:
 *   clang -Wall -Wextra -pedantic -g -o 02_anchored_matching 02_anchored_matching.c && ./02_anchored_matching
 */
#define _POSIX_C_SOURCE 200809L
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
#include <stdbool.h>
#include <stdalign.h>
#ifndef nullptr
#define nullptr NULL
#endif
#endif

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Checks if a character matches regex word class \w (alphanumeric or underscore).
 * Input: char byte.
 * Output: true if [a-zA-Z0-9_], false otherwise.
 * Example: 'a' -> true, '_' -> true, '!' -> false
 */
[[maybe_unused]] static bool is_word_char(char c) {
  (void)c;
  return false; /* TODO: implement */
}

/*
 * Evaluates if pattern matches strictly starting at current text cursor.
 * Input: text cursor pointer, pattern string.
 * Output: true if matched at this exact byte, false otherwise.
 * Example: text="cat", pattern="c" -> true; text="cat", pattern="a" -> false
 */
static bool match_here(const char *text, const char *pattern) {
  (void)text;
  (void)pattern;
  return false; /* TODO: implement */
}

/*
 * Scans input text by stepping cursor forward to find a match anywhere.
 * Input: text string, pattern string.
 * Output: true if match found anywhere, false otherwise.
 * Example: text="apple", pattern="l" -> true
 */
static bool find_pattern(const char *text, const char *pattern) {
  (void)text;
  (void)pattern;
  return false; /* TODO: implement */
}

int main(void) {
  // Test 1: Anchored matching (match_here)
  assert(match_here("dog", "d") == true);
  assert(match_here("dog", "o") == false);
  assert(match_here("dog", "\\w") == true);
  assert(match_here("_private", "\\w") == true);
  assert(match_here("!bang", "\\w") == false);

  // Test 2: Unanchored search across string (find_pattern)
  assert(find_pattern("apple", "p") == true);
  assert(find_pattern("apple", "z") == false);
  assert(find_pattern("---x---", "x") == true);
  assert(find_pattern("$$$9$$$", "\\w") == true);
  assert(find_pattern("$$$$$$$", "\\w") == false);
  assert(find_pattern("", "a") == false);

  printf("[PASS] Exercise 02: Anchored Matching verified.\n");
  return EXIT_SUCCESS;
}
