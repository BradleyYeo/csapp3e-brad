/*
 * Compile and run:
 *   clang -std=c23 -Wall -Wextra -Werror -pedantic -g -o 17_recursion_patterns
 * 17_recursion_patterns.c && ./17_recursion_patterns
 */
#define _POSIX_C_SOURCE 200809L
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
#include <stdalign.h>
#include <stdbool.h>
#ifndef nullptr
#define nullptr NULL
#endif
#endif

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Exercise 1: Linear Pointer Recursion
 *
 * Computes length of null-terminated string recursively.
 * Precondition: s is a non-null pointer to a null-terminated string.
 * Postcondition: returns number of characters before '\0'.
 *
 * Mental Model Questions:
 * - What is the base case where recursion terminates immediately?
 * - If *s != '\0', how do you express total length in terms of s + 1?
 */
static size_t rec_strlen(const char *s) {
  if (*s == '\0') {
    return 0;
  }
  return rec_strlen(s + 1) + 1;
}

/*
 * Exercise 1b: Conditional Linear Pointer Recursion
 *
 * Counts occurrences of target character in null-terminated string s.
 * Precondition: s is a non-null pointer to a null-terminated string.
 * Postcondition: returns number of times target appears in s.
 *
 * Mental Model Questions:
 * - What is the base case when *s == '\0'?
 * - How does the inductive step decide whether to add 1 or 0 to
 * rec_count_char(s + 1, target)?
 */
static size_t rec_count_char(const char *s, char target) {
  // TODO: Implement conditional linear pointer recursion
  if (*s == '\0') {
    return 0;
  }

  return (*s == target) + rec_count_char(s + 1, target);
}

/*
 * Exercise 1c: Linear Pointer Search with Early Termination
 *
 * Tests if target character exists anywhere in null-terminated string s.
 * Precondition: s is a non-null pointer to a null-terminated string.
 * Postcondition: returns true if target is found, false if '\0' is reached
 * first.
 *
 * Mental Model Questions:
 * - What is the failure base case when s has been completely exhausted?
 * - What is the success base case that allows returning true immediately
 * without recursing?
 * - If neither base case is met, what is the recursive call on the remainder of
 * the string?
 */
static bool rec_contains(const char *s, char target) {
  // TODO: Implement linear search with early termination
  if (*s == '\0') {
    return false;
  }
  if (*s == target) {
    return true;
  }
  return rec_contains(s+1, target);
}

/*
 * Exercise 2: Dual-Pointer Lockstep Recursion
 *
 * Tests if text begins with prefix by advancing two pointers simultaneously.
 * Precondition: prefix and text are valid null-terminated strings.
 * Postcondition: returns true if text starts with prefix, false otherwise.
 *
 * Mental Model Questions:
 * - Which string must hit '\0' first for the function to return true? prefix
 * - What should happen if *text == '\0' while *prefix != '\0'?
 * - What should happen if *prefix != *text?
 * - If *prefix == *text, what are the arguments to the recursive call?
 */
static bool rec_starts_with(const char *prefix, const char *text) {
  if (*prefix == '\0') {
    return true;
  }
  if (*prefix == *text) {
    return rec_starts_with(prefix+1, text+1);
  }
  return false;
}

/*
 * Exercise 3: Branching Recursion with Backtracking
 *
 * Tests if text contains one or more 'target' chars followed by 'rest'.
 * Matches regular expression: target+ rest
 * Precondition: rest and text are valid null-terminated strings.
 * Postcondition: returns true if text satisfies target+ rest, false otherwise.
 *
 * Decision Tree Structure:
 *                     rec_match_one_or_more('a', "ts", text)
 *                                       |
 *                     +-----------------+-----------------+
 *                     | (*text != 'a')                    | (*text == 'a')
 *                     v                                   v
 *                return false                       Text matched 1 'a'
 *                                                         |
 *                                       +-----------------+-----------------+
 *                                       | Branch 1 (Greedy)                 |
 * Branch 2 (Fallback) v                                   v Consume another
 * 'a':                  Advance past 'a+': rec_match_one_or_more('a', "ts",
 * text+1)    rec_starts_with("ts", text+1)
 *
 * Mental Model Questions:
 * - Phase 1: If *text != target, why is it impossible for target+ to match?
 * - Phase 2: If *text == target, why must you test Branch 1 first?
 * - If Branch 1 returns false, what fallback function call must you test?
 */
static bool rec_match_one_or_more(char target, const char *rest,
                                  const char *text) {
  // TODO: Implement branching recursion with backtracking
  (void)target;
  (void)rest;
  (void)text;
  return false;
}

int main(void) {
  printf("Running Exercise 17: Recursion Patterns tests...\n\n");

  // --- Exercise 1: rec_strlen ---
  printf("[TEST] Exercise 1: rec_strlen\n");
  assert(rec_strlen("") == 0);
  assert(rec_strlen("a") == 1);
  assert(rec_strlen("grep") == 4);
  assert(rec_strlen("deliberate practice") == 19);
  printf("  -> Passed all rec_strlen tests.\n\n");

  // --- Exercise 1b: rec_count_char ---
  printf("[TEST] Exercise 1b: rec_count_char\n");
  assert(rec_count_char("", 'a') == 0);
  assert(rec_count_char("banana", 'a') == 3);
  assert(rec_count_char("banana", 'b') == 1);
  assert(rec_count_char("banana", 'z') == 0);
  assert(rec_count_char("aaaa", 'a') == 4);
  printf("  -> Passed all rec_count_char tests.\n\n");

  // --- Exercise 1c: rec_contains ---
  printf("[TEST] Exercise 1c: rec_contains\n");
  assert(rec_contains("", 'a') == false);
  assert(rec_contains("banana", 'a') == true);
  assert(rec_contains("banana", 'b') == true);
  assert(rec_contains("banana", 'n') == true);
  assert(rec_contains("banana", 'z') == false);
  printf("  -> Passed all rec_contains tests.\n\n");

  // --- Exercise 2: rec_starts_with ---
  printf("[TEST] Exercise 2: rec_starts_with\n");
  assert(rec_starts_with("", "cat") == true);
  assert(rec_starts_with("cat", "cat") == true);
  assert(rec_starts_with("ca", "cat") == true);
  assert(rec_starts_with("cats", "cat") == false);
  assert(rec_starts_with("dog", "cat") == false);
  assert(rec_starts_with("", "") == true);
  assert(rec_starts_with("a", "") == false);
  printf("  -> Passed all rec_starts_with tests.\n\n");

  // --- Exercise 3: rec_match_one_or_more ---
  printf("[TEST] Exercise 3: rec_match_one_or_more\n");
  assert(rec_match_one_or_more('a', "ts", "cats") == false);
  assert(rec_match_one_or_more('a', "ts", "ats") == true);
  assert(rec_match_one_or_more('a', "ts", "aats") == true);
  assert(rec_match_one_or_more('a', "ts", "aaats") == true);
  assert(rec_match_one_or_more('a', "at", "aaat") == true);
  assert(rec_match_one_or_more('a', "ts", "") == false);
  assert(rec_match_one_or_more('a', "", "a") == true);
  assert(rec_match_one_or_more('a', "", "aaa") == true);
  assert(rec_match_one_or_more('b', "a", "ba") == true);
  assert(rec_match_one_or_more('b', "a", "b") == false);
  printf("  -> Passed all rec_match_one_or_more tests.\n\n");

  printf(
      "[ALL PASSED] Exercise 17: Recursion Patterns successfully verified.\n");
  return EXIT_SUCCESS;
}
