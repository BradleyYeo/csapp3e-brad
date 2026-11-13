# Stage 4: Exercise 13 - Valgrind Architecture and Memory Diagnostics

A guide explaining Valgrind Dynamic Binary Instrumentation (DBI), Memcheck shadow memory (A-bits vs V-bits), uninitialized memory propagation, and the 4 leak categories for C beginners.

# Conceptual Foundations and Mental Model

## Dynamic Binary Instrumentation (DBI)
- Unlike AddressSanitizer (which injects checks at compile time), Valgrind is a virtual machine emulator using Dynamic Binary Instrumentation (DBI).
- Architecture Translation Pipeline:
  - Disassembly: Translates native CPU machine code into an architecture-neutral intermediate representation called VEX IR.
  - Instrumentation: Memcheck injects tracking instructions into the VEX IR.
  - JIT Compilation: Emits transformed native instructions stored in a translation cache.
- Key Advantage: Can analyze third-party closed-source libraries and standard libc without recompiling them with debug flags.
- Performance Cost: 20x to 30x execution slowdown compared to native execution.

## Memcheck Shadow State: A-Bits vs V-Bits
- Memcheck tracks two parallel shadow structures for every byte of memory:
  - Addressability Bits (A-bits):
    - Resolution: 1 bit per byte of memory.
    - Meaning: Does the program have permission to access this address?
    - Set to 1 upon `malloc`; set to 0 upon `free` or in redzones.
    - Violation: Accessing an address with A-bit = 0 triggers `Invalid read of size N` or `Invalid write of size N`.
  - Validity Bits (V-bits):
    - Resolution: 1 bit per bit of data (in both memory and CPU registers).
    - Meaning: Does this bit contain a defined, initialized value?
    - Newly allocated memory has A-bits = 1, but V-bits = 0 (allocated but uninitialized).
    - When a value is written, its V-bits flip to 1.

## Lazy Reporting of Uninitialized Values
- In C, simply copying uninitialized memory (e.g. `int b = a;` where `a` is uninitialized) is NOT reported as an error by Valgrind.
- Memcheck propagates V-bits through registers and ALU operations.
- The error is only triggered when uninitialized data causes an observable effect:
  - Evaluating a conditional branch: `if (b > 0)` (triggers `Conditional jump or move depends on uninitialised value(s)`).
  - Passing as an address or parameter to a system call: `write(fd, &b, 4)` (triggers `Syscall param write(buf) points to uninitialised byte(s)`).

## The 4 Kinds of Memory Leaks
- When a process terminates, Valgrind classifies unfreed heap blocks by inspecting pointer reachability from the root set (stack, registers, global/static data):
  - Definitely Lost: No pointer anywhere points to the start or interior of the block. The memory is impossible to free. Must be fixed immediately.
  - Indirectly Lost: The block is pointed to only by other leaked blocks (e.g. child nodes in an orphaned binary tree or linked list).
  - Possibly Lost: A pointer points into the middle of the block rather than the start (interior pointer). May indicate complex pointer arithmetic or an array slice.
  - Still Reachable: A pointer to the start of the block still exists in a global or stack variable at exit. Often deliberate in short-lived CLI utilities, but represents poor hygiene in long-running servers.

---

# Line-by-Line Code Breakdown

## Structures: SymbolEntry and ScopeEnvironment

```c
typedef struct SymbolEntry {
  char *key;
  char *val;
  size_t val_len;
  int symbol_id;
  int is_constant;
  struct SymbolEntry *next;
} SymbolEntry;

typedef struct ScopeEnvironment {
  int scope_level;
  size_t symbol_count;
  SymbolEntry *head;
  struct ScopeEnvironment *parent;
} ScopeEnvironment;
```

### Concept Design: Clear Ownership Boundaries
- `SymbolEntry`: A heap-allocated key-value node with duplicated strings.
- `ScopeEnvironment`: Owns the linked list of symbol entries in its scope level.
- `parent`: Points to the enclosing outer scope.

---

## Function: symbol_entry_create

```c
static SymbolEntry *symbol_entry_create(const char *key, const char *val, int symbol_id, int is_constant) {
  if (key == nullptr || val == nullptr) {
    return nullptr;
  }

  SymbolEntry *entry = malloc(sizeof(SymbolEntry));
  if (entry == nullptr) {
    return nullptr;
  }

  size_t klen = strlen(key);
  entry->key = malloc(klen + 1);
  if (entry->key == nullptr) {
    free(entry);
    return nullptr;
  }
  memcpy(entry->key, key, klen + 1);

  size_t vlen = strlen(val);
  entry->val = malloc(vlen + 1);
  if (entry->val == nullptr) {
    free(entry->key);
    free(entry);
    return nullptr;
  }
  memcpy(entry->val, val, vlen + 1);

  entry->val_len = vlen;
  entry->symbol_id = symbol_id;
  entry->is_constant = is_constant;
  entry->next = nullptr;

  return entry;
}
```

### Invariant: Full Initialization of All Struct Members
- Every field (`key`, `val`, `val_len`, `symbol_id`, `is_constant`, `next`) is explicitly assigned.
- If any struct member were omitted, its V-bits would remain 0, leading to spurious uninitialized diagnostics when evaluated.
- Defensive unwind: If allocating `entry->val` fails, `entry->key` and `entry` are freed before returning `nullptr`.

---

## Recursive Destruction: scope_env_destroy

```c
static void scope_env_destroy(ScopeEnvironment *env) {
  if (env == nullptr) {
    return;
  }

  SymbolEntry *curr = env->head;
  while (curr != nullptr) {
    SymbolEntry *next = curr->next;
    symbol_entry_destroy(curr);
    curr = next;
  }

  free(env);
}
```

### Order of Deallocation
- Saves `curr->next` before calling `symbol_entry_destroy(curr)` to prevent reading freed memory (Use-After-Free).
- Destroys each child entry, then frees the environment node itself. Leaves zero definitely or indirectly lost blocks.

---

# Memory Layout Visualization

Memcheck A-bits and V-bits across memory states:

```
Memory Address: 0x4000 (Allocated 4 bytes via malloc)
  A-bits: [ 1 ][ 1 ][ 1 ][ 1 ]  -> Addressable! Reads/writes allowed.
  V-bits: [ 0 ][ 0 ][ 0 ][ 0 ]  -> Undefined! Data is garbage.

Operation: int x = *(int *)0x4000;
  V-bits propagate into register: CPU register EAX holds undefined bits.
  Memcheck: NO WARNING YET (lazy propagation).

Operation: if (x > 0) { ... }
  CPU must test undefined bit to make a branch decision!
  Memcheck: DIAGNOSTIC TRIGGERED!
  "Conditional jump or move depends on uninitialised value(s)"

Operation: free(0x4000);
  A-bits: [ 0 ][ 0 ][ 0 ][ 0 ]  -> No longer addressable!
  Attempted read: triggers "Invalid read of size 4"
```

Valgrind 4 Leak Kinds Reachability Graph:

```
Root Set (Global / Stack Pointer):
  [ Root Pointer ]
         |
         v
    +---------+       +---------+
    | Block A | ----> | Block B |       +---------+
    +---------+       +---------+       | Block C |
   (Still Reachable) (Still Reachable)  +---------+
                                      (Definitely Lost)
                                            |
                                            v
                                        +---------+
                                        | Block D |
                                        +---------+
                                     (Indirectly Lost)
```

---

# Active Recall and Validation Drills

## Drill: Active Recall Questions
- Question 1: What is the exact distinction between Addressability (A-bits) and Validity (V-bits)?
- Question 2: Why does Valgrind wait until a conditional branch or system call before warning about uninitialized memory rather than warning on the initial read?
- Question 3: What is the difference between a "Definitely Lost" leak and an "Indirectly Lost" leak?

## Drill: Hands-On Code Validation
- Run [13_valgrind_fundamentals.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/13_valgrind_fundamentals.c) with leak tracking flags:
  ```bash
  clang -std=c23 -Wall -Wextra -Werror -pedantic -g -o 13_valgrind_fundamentals 13_valgrind_fundamentals.c
  ./13_valgrind_fundamentals
  ```
- If running on a Linux system or Docker container with Valgrind installed:
  ```bash
  valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./13_valgrind_fundamentals
  ```
- Confirm that the report outputs:
  `All heap blocks were freed -- no leaks are possible`
  `ERROR SUMMARY: 0 errors from 0 contexts`

---

# Next Step in Curriculum
Proceed to [14_hoare_logic_refactoring.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/14_hoare_logic_refactoring.md) (Stage 5) to learn formal verification, Hoare triples $\{P\}\ C\ \{Q\}$, loop invariants, and Jimmy Koppel's Three Levels of Software.
