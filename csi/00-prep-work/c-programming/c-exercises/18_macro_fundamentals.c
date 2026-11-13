/*
 * Compile and run:
 *   clang -std=c23 -Wall -Wextra -Werror -pedantic -g -o 18_macro_fundamentals 18_macro_fundamentals.c && ./18_macro_fundamentals
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Exercise 1: Operator Precedence & Parenthesization Invariants
 *
 * Mental Model:
 * - Preprocessor operates as pure textual substitution before lexical analysis and AST construction.
 * - Without explicit parentheses, operator precedence in the surrounding code can tear
 *   apart expressions passed as macro arguments, or tear apart the macro's output expression.
 *
 * Invariants:
 * - Rule 1: Every parameter reference inside the macro replacement list must be enclosed in parentheses: (param).
 * - Rule 2: The entire macro replacement expression must be enclosed in outer parentheses: (...).
 */

/* Flawed macro without argument and expression wrapping */
#define BAD_SCALE_OFFSET(base, idx, stride) base + idx * stride

/* Correct macro with full argument and expression parenthesization */
#define SAFE_SCALE_OFFSET(base, idx, stride) (((base) + ((idx) * (stride))))

/*
 * Bit testing macro:
 * Demonstrates operator precedence hazards between bitwise '&' and comparison '!='.
 * In C, '!=' has higher precedence than '&'.
 * Therefore: val & 1U << bit != 0 parses as: val & ((1U << bit) != 0), which degrades to val & 1!
 */
#define BAD_IS_BIT_SET(val, bit) val & (1U << bit) != 0
#define SAFE_IS_BIT_SET(val, bit) ((((val) & (1U << (bit)))) != 0U)

/*
 * Exercise 2: Statement Atomicity & The do { ... } while (0) Semicolon Idiom
 *
 * Mental Model:
 * - A multi-statement macro enclosed merely in '{ ... }' fails when placed in an if/else
 *   statement without braces:
 *     if (cond)
 *         SWAP(x, y, int);  // Semicolon here terminates the if-statement!
 *     else                  // 'else' without a previous 'if' compile error!
 *         ...
 * - Wrapping with 'do { ... } while (0)' guarantees that:
 *   1. The macro forms a single, atomic syntactic statement.
 *   2. The caller MUST provide a trailing semicolon (just like a function call).
 *   3. It introduces a private local block scope for temporary variables.
 */

#define SWAP_TYPED(a, b, type) \
  do {                         \
    type _swap_tmp = (a);      \
    (a) = (b);                 \
    (b) = _swap_tmp;           \
  } while (0)

/*
 * Exercise 3: Preprocessor Operators: Stringification (#) and Token Concatenation (##)
 *
 * Mental Model:
 * - Stringification ('#'): Converts a macro parameter into a string literal at preprocess time.
 * - Token Concatenation ('##'): Pastes two tokens together to produce a single token (identifier/keyword/number).
 * - Two-Level Prescan Rule: If a parameter is an operand to '#' or '##', macro expansion is INHIBITED
 *   for that parameter. To force expansion of a macro argument before stringifying or pasting,
 *   you MUST introduce an intermediate indirection macro.
 */

/* Direct stringification (inhibits prescan) */
#define STR_DIRECT(x) #x

/* Two-level stringification (forces macro prescan expansion) */
#define STR(x) STR_DIRECT(x)

/*
 * Macro-based Code Generator: Generates typed min functions using token pasting.
 * In C, before C11 _Generic or C++ templates, token pasting was the primary mechanism
 * for generic algorithms and typed container instantiation.
 */
#define DEFINE_TYPED_MIN(type, name)                      \
  static inline type min_##name(type a, type b) {         \
    return (a < b) ? a : b;                               \
  }

DEFINE_TYPED_MIN(int, int)
DEFINE_TYPED_MIN(double, dbl)
DEFINE_TYPED_MIN(uint64_t, u64)

/*
 * Exercise 4: Linux Systems & Kernel Idioms (ARRAY_SIZE, offsetof, container_of)
 *
 * Mental Model:
 * - ARRAY_SIZE: Computes array length at compile time.
 *   Hazard: If called on a decayed pointer (e.g. function parameter int *arr),
 *   sizeof(arr)/sizeof(arr[0]) silently produces sizeof(pointer)/sizeof(int) = 8/4 = 2!
 *   Kernel Defense: Use compile-time type verification (__builtin_types_compatible_p).
 *   If type of arr and type of &(arr)[0] match, it is a pointer, not an array.
 *
 * - OFFSETOF: Computes byte offset of a struct field by pretending a struct resides at address 0.
 *   ((size_t)&(((type *)0)->member))
 *
 * - CONTAINER_OF: Given a pointer to an embedded member, recovers the pointer to the containing struct.
 *   Used ubiquitously across the Linux kernel for intrusive linked lists (struct list_head)
 *   and intrusive trees (struct rb_node).
 */

/* Compile-time assertion that evaluates to zero on success and fails compilation on error */
#define BUILD_BUG_ON_ZERO(e) (0 * sizeof(struct { int _dummy : 1 - 2 * (int)!!(e); }))

/* Safe array size with pointer decay prevention */
#define ARRAY_SIZE_SAFE(arr) \
  (sizeof(arr) / sizeof((arr)[0]) + \
   BUILD_BUG_ON_ZERO(__builtin_types_compatible_p(typeof(arr), typeof(&(arr)[0]))))

/*
 * Byte offset of member within struct:
 * Historically implemented in C89 as: ((size_t)&(((type *)0)->member))
 * Modern compilers and sanitizers (UBSan) prefer __builtin_offsetof to avoid
 * triggering runtime null-pointer dereference checks during unevaluated expressions.
 */
#if defined(__has_builtin) && __has_builtin(__builtin_offsetof)
#define MY_OFFSETOF(type, member) __builtin_offsetof(type, member)
#else
#define MY_OFFSETOF(type, member) ((size_t)&(((type *)0)->member))
#endif

/*
 * Intrusive pointer recovery:
 * Computes: (type *)((char *)ptr - offsetof(type, member))
 * Includes type-check temporary pointer to ensure ptr type matches member type.
 */
#define CONTAINER_OF_SAFE(ptr, type, member) __extension__ ({ \
  const typeof(((type *)0)->member) *__mptr = (ptr);          \
  (type *)((char *)__mptr - MY_OFFSETOF(type, member));       \
})

/* Intrusive data structure representation for testing */
struct IntrusiveListNode {
  struct IntrusiveListNode *next;
  struct IntrusiveListNode *prev;
};

struct LinuxProcessTask {
  int pid;
  char comm[16];
  struct IntrusiveListNode run_list;
  uint64_t virtual_runtime_ns;
};

/*
 * Exercise 5: Macro Hygiene, Side-Effect Protection, and Ousterhout/Koppel Inlining Tradeoffs
 *
 * Mental Model:
 * - Side-effect duplication: Passing expressions like x++ to a macro that uses its argument
 *   multiple times evaluates the side-effect multiple times!
 * - Jimmy Koppel & John Ousterhout Maintainability Principle:
 *   Favor 'static inline' functions over macros for general computation.
 *   Use macros ONLY when:
 *   1. Syntactic tokens must be pasted or stringified.
 *   2. Accessing call-site location (__FILE__, __LINE__, __func__).
 *   3. Implementing language-level control extensions (intrusive iterators).
 *   4. Creating compile-time layout calculations (offsetof, container_of, static asserts).
 */

/* Unsafe macro: evaluates arguments twice */
#define UNSAFE_MAX(a, b) ((a) > (b) ? (a) : (b))

/* Safe macro: evaluates arguments exactly once using statement expression and temporary bindings */
#define SAFE_MAX_EXPR(a, b) __extension__ ({ \
  typeof(a) _eval_a = (a);                   \
  typeof(b) _eval_b = (b);                   \
  _eval_a > _eval_b ? _eval_a : _eval_b;     \
})

/* Idiomatic C23 standard inline function alternative */
static inline int safe_max_func(int a, int b) {
  return (a > b) ? a : b;
}

int main(void) {
  printf("Running Exercise 18: Macro Fundamentals tests...\n\n");

  // --- Exercise 1: Operator Precedence & Parenthesization Invariants ---
  printf("[TEST] Exercise 1: Precedence and Parenthesization\n");
  {
    /* BAD_SCALE_OFFSET: 10 + 2 * 4 vs (10 + 2) * 4 */
    int base = 10;
    int idx = 2;
    int stride = 4;
    int safe_result = SAFE_SCALE_OFFSET(base, idx, stride);
    assert(safe_result == 18);

    /* Argument expression tearing hazard: idx passed as (1 + 1), stride as (2 + 2) */
    /* BAD expands to: 10 + 1 + 1 * 2 + 2 = 10 + 1 + 2 + 2 = 15 */
    /* SAFE expands to: (((10) + ((1 + 1) * (2 + 2)))) = 10 + 8 = 18 */
    int bad_teared = BAD_SCALE_OFFSET(base, 1 + 1, 2 + 2);
    int safe_teared = SAFE_SCALE_OFFSET(base, 1 + 1, 2 + 2);
    assert(bad_teared == 15);
    assert(safe_teared == 18);

    /* Outer expression tearing hazard: multiplied by 2 */
    /* BAD: 10 + 2 * 4 * 2 = 10 + 16 = 26 (unexpected) */
    /* SAFE: (((10) + ((2) * (4)))) * 2 = 18 * 2 = 36 (expected) */
    int safe_outer = SAFE_SCALE_OFFSET(base, idx, stride) * 2;
    assert(safe_outer == 36);

    /* Bitwise precedence hazard: BAD_IS_BIT_SET fails for bits > 0 */
    uint32_t val = 0b00000010; /* Bit 1 is set, bit 0 is cleared */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wparentheses"
    /* BAD evaluates: val & ((1U << 1) != 0) -> val & 1 -> 0 (FALSE NEGATIVE!) */
    bool bad_bit1 = BAD_IS_BIT_SET(val, 1);
#pragma GCC diagnostic pop
    bool safe_bit1 = SAFE_IS_BIT_SET(val, 1);
    assert(bad_bit1 == false);
    assert(safe_bit1 == true);

    uint32_t val2 = 0b00000100; /* Bit 2 is set */
    assert(SAFE_IS_BIT_SET(val2, 2) == true);
    assert(SAFE_IS_BIT_SET(val2, 0) == false);
    assert(SAFE_IS_BIT_SET(val2, 1) == false);
    printf("  -> Passed all precedence and parenthesization tests.\n\n");
  }

  // --- Exercise 2: Statement Atomicity & do { ... } while (0) ---
  printf("[TEST] Exercise 2: Statement Atomicity with do { ... } while (0)\n");
  {
    int x = 100;
    int y = 200;

    /* Semicolon safety in unbraced if/else branch */
    bool condition = true;
    if (condition)
      SWAP_TYPED(x, y, int);
    else
      x = 0;

    assert(x == 200);
    assert(y == 100);

    /* Verify else branch execution */
    condition = false;
    if (condition)
      SWAP_TYPED(x, y, int);
    else
      SWAP_TYPED(y, x, int);

    assert(x == 100);
    assert(y == 200);

    /* Verify swap on different data types */
    double da = 3.14;
    double db = 2.71;
    SWAP_TYPED(da, db, double);
    assert(da == 2.71 && db == 3.14);
    printf("  -> Passed all statement atomicity tests.\n\n");
  }

  // --- Exercise 3: Stringification & Token Concatenation ---
  printf("[TEST] Exercise 3: Preprocessor Operators (# and ##)\n");
  {
#define BUFFER_CAPACITY 4096
    /* Direct stringification captures macro identifier text instead of its expanded value */
    const char *direct_str = STR_DIRECT(BUFFER_CAPACITY);
    assert(strcmp(direct_str, "BUFFER_CAPACITY") == 0);

    /* Two-level stringification forces macro argument expansion first */
    const char *expanded_str = STR(BUFFER_CAPACITY);
    assert(strcmp(expanded_str, "4096") == 0);
#undef BUFFER_CAPACITY

    /* Verify generated functions via token concatenation (##) */
    int min_i = min_int(42, 17);
    assert(min_i == 17);

    double min_d = min_dbl(9.81, 3.14);
    assert(min_d == 3.14);

    uint64_t min_u = min_u64(1000ULL, 5000ULL);
    assert(min_u == 1000ULL);
    printf("  -> Passed all stringification and token concatenation tests.\n\n");
  }

  // --- Exercise 4: Linux Systems & Kernel Idioms ---
  printf("[TEST] Exercise 4: Linux Systems Idioms (ARRAY_SIZE, offsetof, container_of)\n");
  {
    /* 1. Compile-time safe array size */
    int stack_array[13];
    size_t count = ARRAY_SIZE_SAFE(stack_array);
    assert(count == 13);

    const char *string_array[4] = {"sys", "kernel", "cgroups", "namespaces"};
    assert(ARRAY_SIZE_SAFE(string_array) == 4);

    /* 2. Byte offset verification */
    size_t pid_offset = MY_OFFSETOF(struct LinuxProcessTask, pid);
    size_t comm_offset = MY_OFFSETOF(struct LinuxProcessTask, comm);
    size_t list_offset = MY_OFFSETOF(struct LinuxProcessTask, run_list);
    size_t runtime_offset = MY_OFFSETOF(struct LinuxProcessTask, virtual_runtime_ns);

    assert(pid_offset == 0);
    assert(comm_offset == sizeof(int));
    assert(list_offset >= comm_offset + 16);
    assert(runtime_offset >= list_offset + sizeof(struct IntrusiveListNode));

    /* 3. Intrusive pointer recovery (container_of) */
    struct LinuxProcessTask task1 = {
      .pid = 1024,
      .comm = "kworker/u:0",
      .run_list = { .next = nullptr, .prev = nullptr },
      .virtual_runtime_ns = 5491204ULL
    };

    /* Extract pointer to intrusive member */
    struct IntrusiveListNode *node_ptr = &task1.run_list;

    /* Recover outer containing struct pointer */
    struct LinuxProcessTask *recovered_task = CONTAINER_OF_SAFE(node_ptr, struct LinuxProcessTask, run_list);

    assert(recovered_task == &task1);
    assert(recovered_task->pid == 1024);
    assert(strcmp(recovered_task->comm, "kworker/u:0") == 0);
    assert(recovered_task->virtual_runtime_ns == 5491204ULL);
    printf("  -> Passed all Linux systems idioms tests.\n\n");
  }

  // --- Exercise 5: Macro Hygiene, Side Effects, and Inlining Tradeoffs ---
  printf("[TEST] Exercise 5: Macro Hygiene & Side-Effect Evaluation\n");
  {
    /* Demonstrating side effect duplication in UNSAFE_MAX */
    int counter_a = 5;
    int counter_b = 2;
    int unsafe_val = UNSAFE_MAX(counter_a++, counter_b);
    /* counter_a was evaluated twice: once in comparison, once in value selection! */
    assert(unsafe_val == 6);
    assert(counter_a == 7); /* Incremented TWICE! */

    /* Demonstrating single evaluation in SAFE_MAX_EXPR */
    counter_a = 5;
    counter_b = 2;
    int safe_val = SAFE_MAX_EXPR(counter_a++, counter_b);
    assert(safe_val == 5);
    assert(counter_a == 6); /* Incremented ONCE! */

    /* Demonstrating single evaluation in standard static inline function */
    counter_a = 5;
    counter_b = 2;
    int func_val = safe_max_func(counter_a++, counter_b);
    assert(func_val == 5);
    assert(counter_a == 6); /* Incremented ONCE! */
    printf("  -> Passed all macro hygiene and side-effect tests.\n\n");
  }

  printf("[ALL PASSED] Exercise 18: Macro Fundamentals successfully verified.\n");
  return EXIT_SUCCESS;
}
