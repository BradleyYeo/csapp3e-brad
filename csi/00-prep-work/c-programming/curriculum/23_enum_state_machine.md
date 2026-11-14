# Stage 2: Exercise 04 - State Modeling with Enums

A beginner's guide explaining C `enum` types, finite state classification, handling errors as data values (Ousterhout's philosophy), and transition logic.

# Conceptual Foundations and Mental Model

## What Is a C Enum?
- An `enum` (enumeration) is a user-defined data type consisting of named integer constants.
- By default in C, the first enumerator has value `0`, and each subsequent enumerator increments by `1`.
- Why use enums instead of magic numbers or raw strings?
  - Type safety: Distinguishes between token categories and search outcomes.
  - Readability: `PATTERN_DIGIT` conveys intent far better than `2`.
  - Compiler exhaustiveness: Compilers can warn if a `switch` statement forgets to handle an enum variant.

## Defining Errors Out of Existence (Ousterhout's Principle)
- Novice code often calls `exit(1)` or crashes when encountering invalid input syntax.
- John Ousterhout (*A Philosophy of Software Design*) advocates "defining errors out of existence" or returning explicit status types.
- By defining `MATCH_SYNTAX_ERROR` as a legitimate variant of `enum MatchResult`, the function cleanly reports invalid patterns to the caller without aborting the process.

---

# Line-by-Line Code Breakdown

## Enum Declarations

```c
enum PatternKind {
  PATTERN_LITERAL,
  PATTERN_DIGIT,
  PATTERN_WORD,
  PATTERN_INVALID,
  PATTERN_WHITESPACE,
};

enum MatchResult {
  MATCH_FOUND,
  MATCH_NOT_FOUND,
  MATCH_SYNTAX_ERROR
};
```

### Enumerator Values
- `PATTERN_LITERAL` = 0, `PATTERN_DIGIT` = 1, `PATTERN_WORD` = 2, `PATTERN_INVALID` = 3, `PATTERN_WHITESPACE` = 4.
- `MATCH_FOUND` = 0, `MATCH_NOT_FOUND` = 1, `MATCH_SYNTAX_ERROR` = 2.

---

## Function: classify_token

```c
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
    if (pattern[1] == 's' && pattern[2] == '\0') {
      return PATTERN_WHITESPACE;
    }
    return PATTERN_INVALID;
  }

  // Single literal character
  if (pattern[1] == '\0') {
    return PATTERN_LITERAL;
  }

  return PATTERN_INVALID;
}
```

### Defensive Input Guards
- `pattern == nullptr || pattern[0] == '\0'`: Returns `PATTERN_INVALID` immediately.

### Escape Sequence Tokenization
- If `pattern[0] == '\\'`, inspects `pattern[1]`:
  - `'d'` followed by `'\0'` classifies as `PATTERN_DIGIT`.
  - `'w'` followed by `'\0'` classifies as `PATTERN_WORD`.
  - `'s'` followed by `'\0'` classifies as `PATTERN_WHITESPACE`.
  - Any unrecognized escape character returns `PATTERN_INVALID`.

### Single Literal Check
- If `pattern[0]` is not `\` and `pattern[1] == '\0'`, the token is a 1-character literal.

---

## Function: test_char

```c
static bool test_char(char c, enum PatternKind kind) {
  unsigned char uc = (unsigned char)c;

  switch (kind) {
  case PATTERN_DIGIT:
    return isdigit(uc);
  case PATTERN_WORD:
    return isalnum(uc) || c == '_';
  case PATTERN_WHITESPACE:
    return isspace(uc);
  case PATTERN_LITERAL:
  case PATTERN_INVALID:
    return false;
  }
}
```

### Exhaustive Switch Dispatch
- Switches on `kind` to route the character to the matching standard predicate (`isdigit`, `isalnum`, `isspace`).
- Casts to `(unsigned char)` for `<ctype.h>` safety.

---

## Function: match_string

```c
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
```

### Two-Phase Execution (Parse then Execute)
- Phase 1 (Lexing / Classification): `classify_token` runs once upfront to determine the token kind.
- Phase 2 (Execution / Search): The loop scans characters in `line` without repeatedly re-parsing the pattern string on every step.

---

# Memory Layout Visualization

State machine classification flow:

```
Pattern String: "\\d"
  Memory: [ '\\' ][ 'd' ][ '\0' ]
             |      |
             v      v
      classify_token()
             |
             +---> pattern[0] == '\\' && pattern[1] == 'd'
             |
             v
      Returns: PATTERN_DIGIT (integer 1)

Line Search: "user_123"
  Cursor 0: 'u' -> test_char('u', PATTERN_DIGIT) -> false
  Cursor 1: 's' -> test_char('s', PATTERN_DIGIT) -> false
  Cursor 2: 'e' -> test_char('e', PATTERN_DIGIT) -> false
  Cursor 3: 'r' -> test_char('r', PATTERN_DIGIT) -> false
  Cursor 4: '_' -> test_char('_', PATTERN_DIGIT) -> false
  Cursor 5: '1' -> test_char('1', PATTERN_DIGIT) -> true!
  Return: MATCH_FOUND
```

---

# Active Recall and Validation Drills

## Drill: Active Recall Questions
- Question 1: What happens if a `switch` statement on an `enum` omits one of the enum values and has no `default` case under `-Wall -Wextra`?
- Question 2: Why is separating token classification (`classify_token`) from token evaluation (`test_char`) beneficial for performance when scanning a 10MB text file?
- Question 3: How does returning `MATCH_SYNTAX_ERROR` instead of calling `abort()` improve library reusability?

## Drill: Hands-On Code Validation
- Open [04_enum_state_machine.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/04_enum_state_machine.c).
- Add a new enum variant `PATTERN_HEX_DIGIT` to `enum PatternKind`.
- Update `classify_token` to accept `"\\x"` for hex digits.
- Update `test_char` to evaluate `isxdigit(uc)`.
- Add test assertions in `main()` verifying that `"0xDEAD"` matches `"\\x"` and `"hello"` does not.
- Compile and run:
  ```bash
  clang -std=c23 -Wall -Wextra -Werror -pedantic -g -o 04_enum_state_machine 04_enum_state_machine.c
  ./04_enum_state_machine
  ```

---

# Next Step in Curriculum
Proceed to [03_getline_ingestion.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/03_getline_ingestion.md) to learn stream ingestion, POSIX `getline(3)`, dynamic buffer resizing, and EOF detection.
