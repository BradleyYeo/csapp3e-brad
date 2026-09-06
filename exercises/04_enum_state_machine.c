#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

enum PatternKind {
  PATTERN_LITERAL,
  PATTERN_DIGIT,
  PATTERN_WORD,
  PATTERN_INVALID
};

enum MatchResult {
  MATCH_FOUND,
  MATCH_NOT_FOUND,
  MATCH_SYNTAX_ERROR
};

/*
 * Classifies a pattern expression into an explicit token kind.
 * Input: pattern string (null-terminated).
 * Output: PatternKind enum value.
 * Example: "\\d" -> PATTERN_DIGIT, "\\w" -> PATTERN_WORD
 */
static enum PatternKind classify_token(const char *pattern) {
  if (pattern == nullptr || pattern[0] == '\0') {
    return PATTERN_INVALID;
  }

  // Handle escape sequences
  if (pattern[0] == '\\') {
    if (pattern[1] == 'd' && pattern[2] == '\0') {
      return PATTERN_DIGIT;
    }
    if (pattern[1] == 'w' && pattern[2] == '\0') {
      return PATTERN_WORD;
    }
    return PATTERN_INVALID;
  }

  // Single literal character
  if (pattern[1] == '\0') {
    return PATTERN_LITERAL;
  }

  return PATTERN_INVALID;
}

/*
 * Evaluates whether a single character satisfies a pattern kind.
 * Input: character byte, classified PatternKind.
 * Output: true if character satisfies token predicate, false otherwise.
 * Example: c='5', kind=PATTERN_DIGIT -> true
 */
static bool test_char(char c, enum PatternKind kind) {
  unsigned char uc = (unsigned char)c;

  switch (kind) {
    case PATTERN_DIGIT:
      return isdigit(uc);

    case PATTERN_WORD:
      return isalnum(uc) || c == '_';

    case PATTERN_LITERAL:
    case PATTERN_INVALID:
      return false;
  }
}

/*
 * Searches text for pattern, defining syntax errors out of existence.
 * Input: line string, pattern string.
 * Output: MatchResult enum (no calls to exit(1)).
 * Example: line="abc", pattern="b" -> MATCH_FOUND
 */
static enum MatchResult match_string(const char *line, const char *pattern) {
  enum PatternKind kind = classify_token(pattern);

  if (kind == PATTERN_INVALID) {
    return MATCH_SYNTAX_ERROR;
  }

  for (const char *cursor = line; *cursor != '\0'; ++cursor) {
    if (kind == PATTERN_LITERAL) {
      if (*cursor == pattern[0]) {
        return MATCH_FOUND;
      }
    } else {
      if (test_char(*cursor, kind)) {
        return MATCH_FOUND;
      }
    }
  }

  return MATCH_NOT_FOUND;
}

int main(void) {
  // Test 1: Pattern classification
  assert(classify_token("\\d") == PATTERN_DIGIT);
  assert(classify_token("\\w") == PATTERN_WORD);
  assert(classify_token("x") == PATTERN_LITERAL);
  assert(classify_token("\\q") == PATTERN_INVALID);
  assert(classify_token("") == PATTERN_INVALID);
  assert(classify_token(nullptr) == PATTERN_INVALID);

  // Test 2: Matching with explicit result enums
  assert(match_string("user_123", "\\d") == MATCH_FOUND);
  assert(match_string("no_digits", "\\d") == MATCH_NOT_FOUND);
  assert(match_string("!@#", "\\w") == MATCH_NOT_FOUND);
  assert(match_string("hello_world", "_") == MATCH_FOUND);

  // Test 3: Syntax error handled gracefully without crashing
  assert(match_string("any text", "\\unsupported") == MATCH_SYNTAX_ERROR);

  printf("[PASS] Exercise 04: State Modeling with Enums verified.\n");
  return EXIT_SUCCESS;
}
