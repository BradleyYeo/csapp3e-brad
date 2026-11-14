/*
 * Diagnostic 5: C Heap Memory Management Fundamentals (malloc, realloc, free)
 * Based on CS61C Heap Notes: https://notes.cs61c.org/content/c-memory-management/heap/
 *
 * Protocol (see ../README.md):
 *   1. Answer all Predict-Before-Running questions in the comment block below.
 *   2. Implement the 5 TODO functions according to their specifications.
 *   3. Build & run tests + sanitizers + leak checks:
 *        make -C diagnostic test
 *        make -C diagnostic test-san
 *        make -C diagnostic test-leaks   (macOS) / valgrind (Linux)
 */

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
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
 * ============================================================================
 * PART A: PREDICT-BEFORE-RUNNING DIAGNOSTIC QUESTIONS
 * Write your answers below each question before implementing Part B.
 * ============================================================================
 *
 * Q1. Stack vs Heap Lifetime & Sizing
 *   int *make_bad(void)  { int a[4] = {1, 2, 3, 4}; return a; }
 *   int *make_good(void) { int *a = malloc(4); ... return a; }
 *   (a) What happens when the caller dereferences make_bad()? Why?
 *   (b) What bug is hidden in malloc(4) inside make_good() on a 64-bit LP64 machine?
 *       How should the allocation size be written portably?
 *   Your answer:
 *
 * Q2. Uninitialized Heap Memory (malloc vs calloc)
 *   Suppose int *p = malloc(5 * sizeof(*p)); succeeds.
 *   What values are in p[0..4] right after malloc returns?
 *   Why does reading p[0] before writing it cause Undefined Behavior (and a Valgrind V-bit error)?
 *   When does calloc(5, sizeof(*p)) differ, and what extra safety check does calloc perform?
 *   Your answer:
 *
 * Q3. realloc Mechanics & Pitfalls
 *   Consider:
 *     int *a = malloc(4 * sizeof(int));
 *     int *alias = &a[2];
 *     a = realloc(a, 1024 * sizeof(int));
 *   Identify BOTH bugs in those 3 lines:
 *   (a) What happens to `a` (and the original 16 bytes) if realloc returns NULL?
 *   (b) Even if realloc succeeds, why is `*alias` potentially a dangling reference?
 *   (c) What does realloc(NULL, n) do? What is dangerous about realloc(p, 0)?
 *   Your answer:
 *
 * Q4. free() Contract & Undefined Behavior
 *   Predict what happens for each call (assume p = malloc(16)):
 *   (a) free(NULL);
 *   (b) int x = 5; free(&x);
 *   (c) free((char *)p + 4);
 *   (d) free(p); free(p);
 *   (e) free(p); *p = 42;
 *   Your answer:
 */

/*
 * ============================================================================
 * PART B: HANDS-ON HEAP EXERCISES (IMPLEMENT THE 5 TODO FUNCTIONS)
 * ============================================================================
 */

/*
 * Exercise 1: Safe Integer Array Allocation (malloc + overflow guard + init)
 *
 * Requirements:
 * - If count == 0, return nullptr (avoid zero-size malloc ambiguity).
 * - Guard against size_t multiplication overflow:
 *     if (count > SIZE_MAX / sizeof(int)) return nullptr;
 * - Allocate count * sizeof(int) bytes using malloc.
 * - Return nullptr if malloc fails.
 * - Initialize all count elements to fill_value (malloc leaves garbage!).
 * - Return the heap pointer (caller owns it and must free it).
 */
static int *heap_alloc_ints(size_t count, int fill_value) {
  (void)count;
  (void)fill_value;
  return nullptr; /* TODO: implement */
}

/*
 * Exercise 2: Safe Reallocation via Double Pointer (realloc)
 *
 * Requirements:
 * - Return false if arr == nullptr || cap == nullptr.
 * - If new_cap == 0:
 *     free(*arr);
 *     *arr = nullptr;
 *     *cap = 0;
 *     return true;
 *   (Handles shrink-to-zero cleanly without calling implementation-defined realloc(p, 0).)
 * - Guard against multiplication overflow:
 *     if (new_cap > SIZE_MAX / sizeof(int)) return false;
 * - Call realloc(*arr, new_cap * sizeof(int)) into a temporary pointer `tmp`.
 *   Note: if *arr == nullptr, realloc(nullptr, bytes) behaves like malloc(bytes).
 * - If tmp == nullptr:
 *     Return false WITHOUT modifying *arr or *cap (original block preserved!).
 * - If new_cap > *cap, zero-initialize the newly grown elements [*cap .. new_cap - 1]
 *   so no caller reads uninitialized heap bytes.
 * - Update *arr = tmp and *cap = new_cap, then return true.
 */
static bool heap_resize_ints(int **arr, size_t *cap, size_t new_cap) {
  (void)arr;
  (void)cap;
  (void)new_cap;
  return false; /* TODO: implement */
}

/*
 * Exercise 3: Idempotent Deallocation (free + nullify)
 *
 * Requirements:
 * - If ptr == nullptr, return immediately.
 * - Call free(*ptr) (safe even if *ptr == nullptr, since free(nullptr) is a no-op).
 * - Set *ptr = nullptr so any future free or dereference cannot become a double-free
 *   or silent use-after-free.
 */
static void heap_safe_free(void **ptr) {
  (void)ptr;
  /* TODO: implement */
}

/*
 * Exercise 4: Dynamic Append Without Invalidating an Interior Cursor
 *
 * Demonstrates why realloc can turn raw interior pointers into dangling references,
 * and how rebasing by index offset prevents it.
 *
 * Requirements:
 * - Return false if arr == nullptr || len == nullptr || cap == nullptr.
 * - If cursor_inout != nullptr && *cursor_inout != nullptr:
 *     Verify *arr != nullptr and *cursor_inout points to an element in [*arr, *arr + *len).
 *     (Use uintptr_t range check + alignment check to avoid UB comparing unrelated
 *      pointers: (uintptr_t)c >= (uintptr_t)base && (uintptr_t)c < (uintptr_t)(base + *len)
 *      and (((uintptr_t)c - (uintptr_t)base) % sizeof(int)) == 0.)
 *     If invalid, return false.
 *     Record its element index: size_t cursor_idx = (size_t)(*cursor_inout - *arr);
 * - If *len == *cap:
 *     Compute new_cap = (*cap == 0) ? 2 : (*cap > SIZE_MAX / 2 ? 0 : *cap * 2);
 *     If new_cap == 0, return false.
 *     Resize using heap_resize_ints(arr, cap, new_cap); return false on failure.
 *     (Notice: even if realloc moved *arr, cursor_idx stays valid!)
 * - Append (*arr)[*len] = value; (*len)++;
 * - If cursor_inout != nullptr && *cursor_inout != nullptr:
 *     Rebase *cursor_inout = *arr + cursor_idx;
 * - Return true.
 */
static bool heap_append_preserving_cursor(int **arr, size_t *len, size_t *cap,
                                          int value, const int **cursor_inout) {
  (void)arr;
  (void)len;
  (void)cap;
  (void)value;
  (void)cursor_inout;
  return false; /* TODO: implement */
}

/*
 * Exercise 5: Independent Heap Slice Clone (Deep Copy)
 *
 * Requirements:
 * - Return nullptr if src == nullptr || start >= end.
 * - Allocate (end - start) * sizeof(int) bytes via malloc (with overflow check).
 * - Copy elements src[start .. end - 1] into the new buffer using memcpy.
 * - Return the newly allocated buffer (caller owns and frees it).
 */
static int *heap_clone_range(const int *src, size_t start, size_t end) {
  (void)src;
  (void)start;
  (void)end;
  return nullptr; /* TODO: implement */
}

/*
 * ============================================================================
 * PART C: DIAGNOSTIC TEST SUITE
 * ============================================================================
 */

static void test_alloc_ints(void) {
  assert(heap_alloc_ints(0, 7) == nullptr);
  assert(heap_alloc_ints((SIZE_MAX / sizeof(int)) + 1, 7) == nullptr);

  int *nums = heap_alloc_ints(5, 42);
  assert(nums != nullptr);
  for (size_t i = 0; i < 5; i++) {
    assert(nums[i] == 42);
  }
  free(nums);
}

static void test_resize_ints(void) {
  assert(heap_resize_ints(nullptr, nullptr, 4) == false);

  int *arr = nullptr;
  size_t cap = 0;

  /* realloc(nullptr, n) acts like malloc(n) */
  assert(heap_resize_ints(&arr, &cap, 3) == true);
  assert(arr != nullptr && cap == 3);
  for (size_t i = 0; i < cap; i++) {
    assert(arr[i] == 0);
  }
  arr[0] = 10;
  arr[1] = 20;
  arr[2] = 30;

  /* Grow: existing elements preserved, tail zero-initialized */
  assert(heap_resize_ints(&arr, &cap, 6) == true);
  assert(arr != nullptr && cap == 6);
  assert(arr[0] == 10 && arr[1] == 20 && arr[2] == 30);
  assert(arr[3] == 0 && arr[4] == 0 && arr[5] == 0);

  /* Overflow / failed resize must preserve original pointer & data */
  int *saved_ptr = arr;
  size_t huge_cap = (SIZE_MAX / sizeof(int)) + 1;
  assert(heap_resize_ints(&arr, &cap, huge_cap) == false);
  assert(arr == saved_ptr && cap == 6);
  assert(arr[0] == 10 && arr[2] == 30);

  /* Shrink: prefix preserved */
  assert(heap_resize_ints(&arr, &cap, 2) == true);
  assert(arr != nullptr && cap == 2);
  assert(arr[0] == 10 && arr[1] == 20);

  /* Resize to 0: frees block and resets pointer/capacity */
  assert(heap_resize_ints(&arr, &cap, 0) == true);
  assert(arr == nullptr && cap == 0);
}

static void test_safe_free(void) {
  heap_safe_free(nullptr);

  void *p = nullptr;
  heap_safe_free(&p);
  assert(p == nullptr);

  void *x = malloc(sizeof(int));
  assert(x != nullptr);
  *(int *)x = 99;
  heap_safe_free(&x);
  assert(x == nullptr);

  /* Second call must be a safe no-op (no double free) */
  heap_safe_free(&x);
  assert(x == nullptr);
}

static void test_append_preserving_cursor(void) {
  int *arr = nullptr;
  size_t len = 0;
  size_t cap = 0;

  assert(heap_append_preserving_cursor(&arr, &len, &cap, 100, nullptr) == true);
  assert(heap_append_preserving_cursor(&arr, &len, &cap, 200, nullptr) == true);
  assert(len == 2 && cap == 2);

  /* Cursor tracks index 1 (value 200) across many reallocations */
  const int *cursor = &arr[1];
  for (int v = 300; v <= 10000; v += 100) {
    assert(heap_append_preserving_cursor(&arr, &len, &cap, v, &cursor) == true);
    assert(cursor == arr + 1);
    assert(*cursor == 200);
  }

  /* Out-of-bounds cursor must be rejected cleanly */
  int stack_val = 5;
  const int *bad_cursor = &stack_val;
  assert(heap_append_preserving_cursor(&arr, &len, &cap, 999, &bad_cursor) == false);

  free(arr);
  arr = nullptr;
}

static void test_clone_range(void) {
  int src[] = {10, 20, 30, 40, 50};
  assert(heap_clone_range(nullptr, 0, 2) == nullptr);
  assert(heap_clone_range(src, 3, 3) == nullptr);
  assert(heap_clone_range(src, 4, 2) == nullptr);

  int *slice = heap_clone_range(src, 1, 4);
  assert(slice != nullptr);
  assert(slice != src + 1);
  assert(slice[0] == 20 && slice[1] == 30 && slice[2] == 40);

  /* Independent lifecycle: mutating src does not mutate slice */
  src[2] = -999;
  assert(slice[1] == 30);

  free(slice);
  slice = nullptr;
}

int main(void) {
  test_alloc_ints();
  test_resize_ints();
  test_safe_free();
  test_append_preserving_cursor();
  test_clone_range();

  printf("[PASS] Diagnostic 5: C Heap Fundamentals (malloc, realloc, free) verified.\n");
  return EXIT_SUCCESS;
}
