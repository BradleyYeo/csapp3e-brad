# Stage 1: Exercise 05 - Character Group Slicing and Buffer Bounds

A foundational guide explaining pointer subtraction, output parameters via double pointers (`**`), bounded buffer slicing (`memcpy`), and null-termination invariants for C beginners.

# Conceptual Foundations and Mental Model

## Pointer Subtraction and Distance
- In C, subtracting two pointers that point into the same array yields the number of elements between them (a signed integer of type `ptrdiff_t`).
- If `start` points to index 1 and `end` points to index 4:
  ```c
  ptrdiff_t diff = end - start; // Evaluates to 3
  ```
- This is pointer distance arithmetic. The compiler divides the raw byte distance by `sizeof(*pointer)`. For `char*`, since `sizeof(char) == 1`, pointer distance equals the exact byte count.

## Output Parameters and Double Pointers (const char **)
- In C, all function arguments are passed by value (copied).
- If a function needs to modify a caller's pointer variable (e.g. set `start_out` to point to the start of a slice), the caller must pass the address of its pointer: `&my_pointer`.
- The receiving function accepts `const char **start_out` and dereferences it to write back: `*start_out = start;`.

## The Raw Byte Copy Invariant (memcpy vs strncpy)
- `memcpy(dest, src, n)` copies exactly `n` raw bytes from `src` to `dest`. It does NOT append a null terminator (`'\0'`).
- If `dest` is subsequently treated as a C string without explicit null-termination, functions like `printf("%s")` or `strlen()` will read past the end of the buffer into adjacent stack memory (buffer overrun).
- Rule: Whenever slicing a substring into a fixed buffer of capacity `C`, you must ensure `len < C` and immediately assign `dest[len] = '\0'`.

---

# Line-by-Line Code Breakdown

## Function: get_group_bounds

```c
static bool get_group_bounds(const char *pattern, const char **start_out, size_t *len_out) {
  if (pattern == nullptr || pattern[0] != '[') {
    return false;
  }

  const char *start = pattern + 1;
  if (*start == '^') {
    start++;
  }

  const char *end = strchr(start, ']');

  if (end == nullptr) {
    return false;
  }

  *start_out = start;
  *len_out = (size_t)(end - start);
  return true;
}
```

### Precondition Check: pattern[0] != '['
- Verifies that the pattern begins with the opening delimiter `'['`.
- Rejects `nullptr` to avoid segmentation faults.

### Delimiter Skipping and Negation: start = pattern + 1
- Skips past `'['` to the first character of the group.
- If `*start == '^'`, advances `start` again by 1 byte to support negated character classes `[^abc]`.

### Search for Closing Delimiter: strchr(start, ']')
- `strchr` scans memory forward from `start` searching for the first byte matching `']'`.
- Returns a pointer to the matching byte, or `nullptr` if the delimiter is missing.

### Pointer Distance Assignment
- `*start_out = start;`: Stores the start address back to the caller's pointer.
- `*len_out = (size_t)(end - start);`: Computes slice length via pointer subtraction.

---

## Function: slice_to_buffer

```c
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
```

### Capacity Guard: if (len >= capacity) return false
- To store `len` characters plus the terminating `'\0'`, the destination requires `len + 1` bytes.
- Therefore, if `len >= capacity`, writing the null byte would write beyond the buffer boundary (an off-by-one stack buffer overflow).
- Checking `len >= capacity` guarantees memory safety.

### Raw Copy and Explicit Termination
- `memcpy(dest, src, len)`: Copies the raw slice bytes.
- `dest[len] = '\0'`: Guarantees that `dest` is a valid, null-terminated C string.

---

## Function: match_pos_bracket_grp

```c
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
```

### Stack Allocation of Slicing Buffer
- `char charset[256];`: Allocates a fixed 256-byte scratch buffer directly on the thread stack (fast, zero heap fragmentation).
- Bounds extraction and buffer slicing are composed safely before matching.

---

# Memory Layout Visualization

Pointer arithmetic during `get_group_bounds("[abc]", &start, &len)`:

```
Memory:
  Address:  0x100  0x101  0x102  0x103  0x104  0x105
  Byte:    [ '[' ][ 'a' ][ 'b' ][ 'c' ][ ']' ][ '\0' ]
             ^      ^                   ^
             |      |                   |
          pattern  start               end
                   (0x101)             (0x104)

Distance calculation:
  end - start = 0x104 - 0x101 = 3 bytes
  *start_out  = 0x101
  *len_out    = 3
```

Buffer layout during `slice_to_buffer(start, 3, dest, 8)`:

```
Stack Buffer dest (capacity 8):
  Byte 0   Byte 1   Byte 2   Byte 3   Byte 4   Byte 5   Byte 6   Byte 7
  [ 'a' ]  [ 'b' ]  [ 'c' ]  [ '\0' ] [ uninit uninit uninit uninit ]
  \_______________________/     ^
     memcpy(dest, src, 3)    dest[3] = '\0'
```

---

# Active Recall and Validation Drills

## Drill: Active Recall Questions
- Question 1: What is the return type of pointer subtraction (`end - start`), and why must it be signed?
- Question 2: Why will `printf("%s\n", dest)` corrupt memory or print garbage if you forget `dest[len] = '\0'` after `memcpy`?
- Question 3: Why does `slice_to_buffer` check `len >= capacity` instead of `len > capacity`?

## Drill: Hands-On Code Validation
- Open [05_char_group_slicing.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/05_char_group_slicing.c).
- Implement a character-range expansion helper `expand_range(const char *raw_group, char *dest, size_t dest_cap)` that expands ranges like `a-z` or `0-9` into all enclosed characters.
- Add assertions in `main()` verifying that `"[0-3]"` matches `"2"` and does not match `"5"`.
- Compile and run:
  ```bash
  clang -std=c23 -Wall -Wextra -Werror -pedantic -g -o 05_char_group_slicing 05_char_group_slicing.c
  ./05_char_group_slicing
  ```

---

# Next Step in Curriculum
Proceed to [08_pointer_arithmetic_strides.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/08_pointer_arithmetic_strides.md) to master `void*` generic arithmetic, byte offsets, and stride calculations.
