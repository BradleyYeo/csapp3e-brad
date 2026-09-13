# Stage 1: Exercise 02 - Anchored Matching and Character Classes

A foundational guide explaining anchored pattern matching (`match_here`), unanchored search loops (`find_pattern`), character class predicates (`\w`), and cursor advancement.

# Conceptual Foundations and Mental Model

## Anchored vs Unanchored Matching
- Anchored matching: Tests whether a pattern matches text starting at one exact memory address (e.g. index 0 or current cursor `p`). If the first byte does not match, it immediately fails.
- Unanchored matching: Slides a cursor byte-by-byte across the entire string buffer. At each byte address, it invokes the anchored matcher. If any address matches, the search succeeds.
- The separation of concerns:
  - `match_here(cursor, pattern)` handles the single-point match predicate.
  - `find_pattern(text, pattern)` handles the buffer traversal loop.

## Character Classes and Escape Sequences
- In regular expressions, a character class matches any character belonging to a predefined set.
- The class `\w` matches word characters: ASCII letters (`a-z`, `A-Z`), digits (`0-9`), and underscores (`_`).
- C string escape mechanics:
  - In a C string literal, a backslash is an escape character (e.g. `\n`, `\t`).
  - To pass a literal backslash followed by 'w' in C code, you must write `"\\w"`.
  - In memory, `"\\w"` occupies 3 bytes: `0x5C` (`'\\'`), `0x77` (`'w'`), and `0x00` (`'\0'`).

---

# Line-by-Line Code Breakdown

## Function: is_word_char

```c
static bool is_word_char(char c) {
  unsigned char uc = (unsigned char)c;
  return isalnum(uc) || c == '_';
}
```

### Unsigned Character Cast
- In C, the `char` type can be signed or unsigned depending on the platform and architecture.
- The `<ctype.h>` functions (such as `isalnum`) require their argument to be representable as an `unsigned char` or equal to `EOF`.
- If a negative signed `char` (e.g. `(char)0x80` = -128) is passed directly to `isalnum`, it triggers undefined behavior.
- Casting to `(unsigned char)c` guarantees a valid value in the range `0` to `255`.

### Predicate Logic
- `isalnum(uc)` evaluates to non-zero if `uc` is `0-9`, `a-z`, or `A-Z`.
- `c == '_'` handles the underscore character.
- The logical OR (`||`) short-circuits: if `isalnum(uc)` is true, the equality check is skipped.

---

## Function: match_here

```c
static bool match_here(const char *text, const char *pattern) {
  if (pattern[0] == '\0') {
    return true;
  }

  if (text[0] == '\0') {
    return false;
  }

  // Handle \w escape sequence
  if (pattern[0] == '\\' && pattern[1] == 'w') {
    return is_word_char(text[0]);
  }

  // Literal character match
  return text[0] == pattern[0];
}
```

### Base Case 1: Empty Pattern
- `if (pattern[0] == '\0') return true;`
- An empty pattern matches any text (even empty text). This is an essential base case for inductive matching algorithms.

### Base Case 2: Empty Text with Non-Empty Pattern
- `if (text[0] == '\0') return false;`
- If the text has reached the null terminator but the pattern still requires characters, matching fails.

### Multi-Byte Token Detection
- `if (pattern[0] == '\\' && pattern[1] == 'w')`
- Checks whether the pattern cursor currently points to a two-byte escape sequence token.
- If true, delegates character validation to `is_word_char(text[0])`.

### Single Character Equality
- `return text[0] == pattern[0];`
- Fallback for literal characters: directly compares the byte at `text` with the byte at `pattern`.

---

## Function: find_pattern

```c
static bool find_pattern(const char *text, const char *pattern) {
  for (const char *cursor = text; *cursor != '\0'; ++cursor) {
    if (match_here(cursor, pattern)) {
      return true;
    }
  }

  return false;
}
```

### Loop Invariant and Cursor Stepping
- `const char *cursor = text`: Initializes pointer `cursor` to the start of `text`.
- `*cursor != '\0'`: Loops until reaching the null terminator of `text`.
- `++cursor`: Moves `cursor` forward by 1 byte in each iteration.
- `if (match_here(cursor, pattern)) return true;`: Stops immediately upon the first match.
- If the loop finishes without finding a match, returns `false`.

---

# Memory Layout Visualization

Sliding cursor search in `find_pattern("apple", "p")`:

```
Iteration 0:
  text:      [ 'a' ][ 'p' ][ 'p' ][ 'l' ][ 'e' ][ '\0' ]
  cursor:       ^
  match_here("apple", "p"):
    text[0] ('a') == pattern[0] ('p') -> false

Iteration 1:
  cursor:             ^
  match_here("pple", "p"):
    text[0] ('p') == pattern[0] ('p') -> true!
  Return: true immediately (short-circuit)
```

Character class match in `match_here("_private", "\\w")`:

```
  text:         [ '_' ][ 'p' ][ 'r' ][ 'i' ][ 'v' ][ 'a' ][ 't' ][ 'e' ][ '\0' ]
  text[0]:        ^
  pattern:      [ '\\' ][ 'w' ][ '\0' ]
  pattern[0..1]:   ^      ^
  is_word_char('_'):
    isalnum('_') -> false
    '_' == '_'   -> true
  Return: true
```

---

# Active Recall and Validation Drills

## Drill: Active Recall Questions
- Question 1: Why does `match_here` check `pattern[0] == '\0'` before checking `text[0] == '\0'`? What would break if the order were inverted?
- Question 2: Why must literal backslashes be escaped as `"\\w"` in C string literals?
- Question 3: How does `find_pattern` behave when `text` is `""` (empty string) and `pattern` is `"a"`? Trace the loop condition.

## Drill: Hands-On Code Validation
- Open [02_anchored_matching.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/02_anchored_matching.c).
- Add support for the digit class `\d` (matches `0-9` using `isdigit((unsigned char)c)`).
- Update `match_here` to inspect `pattern[0] == '\\' && pattern[1] == 'd'`.
- Add test assertions in `main()`:
  - `assert(match_here("7days", "\\d") == true);`
  - `assert(match_here("days7", "\\d") == false);`
  - `assert(find_pattern("user_42", "\\d") == true);`
- Compile and run:
  ```bash
  clang -std=c23 -Wall -Wextra -Werror -pedantic -g -o 02_anchored_matching 02_anchored_matching.c
  ./02_anchored_matching
  ```

---

# Next Step in Curriculum
Proceed to [05_char_group_slicing.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/05_char_group_slicing.md) to learn stack buffer slicing, bracket character groups (`[abc]`), and safe bounds checking.
