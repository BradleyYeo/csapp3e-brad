# Stage 1: Exercise 01 - Pointer Traversal and Cursor Arithmetic

A foundational guide explaining pointer mechanics, memory dereferencing, boundary invariants, and string traversal for C beginners.

# Conceptual Foundations and Mental Model

## What Is a Pointer?
- A pointer is a variable that stores a memory address where data lives, rather than storing the data value directly.
- In 64-bit systems, all pointers occupy exactly 8 bytes of memory regardless of whether they point to a `char` (1 byte), an `int` (4 bytes), or a large struct.
- Syntax:
  - Declaration: `const char *p;` declares a pointer variable `p` that points to a read-only character.
  - Address-of operator (`&`): Retrieves the memory address of an existing variable (`&var`).
  - Dereference operator (`*`): Retrieves the value stored at the address (`*p`).

## Strings in C Memory
- C has no native "string" object type. A string is simply a contiguous array of byte characters terminated by a null byte (`'\0'`, ASCII integer value 0).
- Memory representation of `"cat"`:
  ```
  Byte Index:     [  0  ]   [  1  ]   [  2  ]   [  3  ]
  Character:        'c'       'a'       't'      '\0'
  Memory Address:  0x1000    0x1001    0x1002    0x1003
  ```
- Because C strings carry no stored length metadata, string algorithms must scan memory forward until encountering `*p == '\0'`.

## Cursor Traversal vs Array Indexing
- Array indexing (`text[i]`): Calculates `*(text + i)` relative to base pointer `text` on every iteration.
- Cursor traversal (`const char *p = text; ++p`): Increments a single hardware register holding the address by 1 byte. Directly maps to single-instruction pointer advancement in assembly.

---

# Line-by-Line Code Breakdown

## Function: count_digits

```c
static size_t count_digits(const char *text) {
  size_t count = 0;

  for (const char *p = text; *p != '\0'; ++p) {
    if (isdigit((unsigned char)*p)) {
      count++;
    }
  }

  return count;
}
```

### Keyword static
- Restricts the function to file scope (internal linkage). Prevents name collisions across C source files.

### Type size_t
- The standard unsigned integer type for counts and memory sizes. Cannot be negative.

### Loop Header: for (const char *p = text; *p != '\0'; ++p)
- Initialization: `const char *p = text` initializes pointer cursor `p` to the memory address of the first character.
- Condition: `*p != '\0'` dereferences `p`. The loop continues as long as the character at `p` is not the null terminator.
- Increment: `++p` advances the pointer to the next memory address (adds `sizeof(char)` = 1 byte).

### Casting to (unsigned char)*p
- `isdigit()` expects an integer representing an `unsigned char` or `EOF`. Casting avoids undefined behavior if `char` is signed and negative.

---

## Function: find_last_digit (Reverse Traversal)

```c
static const char *find_last_digit(const char *text) {
  if (text == nullptr || *text == '\0') {
    return nullptr;
  }
  const char *p = text;
  while (*p != '\0') {
    ++p;
  }
  while (p > text) {
    --p;
    if (isdigit((unsigned char)*p)) {
      return p;
    }
  }
  return nullptr;
}
```

### Precondition Guard: if (text == nullptr || *text == '\0')
- Defensive check: Rejects null pointers and empty strings before any pointer operations.

### Forward Scanning: while (*p != '\0') ++p;
- Advances pointer `p` to point directly to the terminal null byte `'\0'`.

### Lower-Bound Invariant: while (p > text)
- Lower-bound protection: Ensures pointer `p` never decrements before the start of the string (`text`).
- In standard C, decrementing a pointer past the beginning of an allocated array is undefined behavior.

---

## Function: match_prefix (Two-Pointer Lockstep Traversal)

```c
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
```

### Dual Cursor Advancement
- Pointer `s` walks through `text`; pointer `prefix` walks through `prefix`.
- Guard `*prefix != '\0'` ensures the loop stops immediately once the entire prefix has matched, avoiding scanning off the end of `prefix`.

---

# Memory Layout Visualization

Two-pointer lockstep match for `match_prefix("apple", "app")`:

```
Iteration 0:
  text:    [ 'a' ][ 'p' ][ 'p' ][ 'l' ][ 'e' ][ '\0' ]
  cursor s:   ^
  prefix:  [ 'a' ][ 'p' ][ 'p' ][ '\0' ]
  cursor p:   ^
  Match: 'a' == 'a' -> advance both

Iteration 1:
  cursor s:         ^
  cursor p:         ^
  Match: 'p' == 'p' -> advance both

Iteration 2:
  cursor s:               ^
  cursor p:               ^
  Match: 'p' == 'p' -> advance both

Iteration 3:
  cursor s:                     ^
  cursor p:                           ^ (*prefix == '\0')
  Termination: *prefix == '\0' evaluates true -> returns true!
```

---

# Active Recall and Validation Drills

## Drill: Active Recall Questions
- Question 1: What is the numerical value of the null terminator character `'\0'` in C?
- Question 2: What is the exact difference between `*p++`, `(*p)++`, and `*++p`?
- Question 3: Why is testing `p > text` strictly required in backward traversal? What happens if you test `p >= text`?

## Drill: Hands-On Code Validation
- Open [01_pointer_traversal.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/01_pointer_traversal.c).
- Implement a new helper function `count_whitespace(const char *text)` using pure pointer traversal (`isspace((unsigned char)*p)`).
- Add unit assertions in `main()` verifying behavior on `"hello world\t\n"`.
- Compile and verify:
  ```bash
  clang -std=c23 -Wall -Wextra -Werror -pedantic -g -o 01_pointer_traversal 01_pointer_traversal.c
  ./01_pointer_traversal
  ```

---

# Next Step in Curriculum
Proceed to [02_anchored_matching.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/02_anchored_matching.md) to apply cursor pointers to anchored regex matching (`^pattern`).
