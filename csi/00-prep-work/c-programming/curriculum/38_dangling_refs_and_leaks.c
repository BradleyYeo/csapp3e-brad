/*
 * Exercise 52: Identifying, Fixing, and Preventing Dangling References & Memory Leaks
 *
 * Compile and run:
 *   make -C curriculum 52_dangling_refs_and_leaks && ./curriculum/52_dangling_refs_and_leaks
 *
 * Run with Sanitizers (ASan + UBSan):
 *   clang -std=c23 -Wall -Wextra -pedantic -Werror -g -fsanitize=address,undefined \
 *     -fno-omit-frame-pointer -o 52_dangling_refs_and_leaks 52_dangling_refs_and_leaks.c
 *   ./52_dangling_refs_and_leaks
 *
 * Trigger deliberate defects to study sanitizer / leak reports:
 *   ./52_dangling_refs_and_leaks --trigger-dangling-uaf
 *   ./52_dangling_refs_and_leaks --trigger-dangling-realloc
 *   ./52_dangling_refs_and_leaks --trigger-dangling-alias
 *   ./52_dangling_refs_and_leaks --trigger-leak-overwrite
 *   ./52_dangling_refs_and_leaks --trigger-leak-early-return
 *   ./52_dangling_refs_and_leaks --trigger-leak-indirect
 *
 * Check leaks (macOS / Linux):
 *   leaks --atExit -- ./52_dangling_refs_and_leaks
 *   valgrind --leak-check=full --show-leak-kinds=all ./52_dangling_refs_and_leaks
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
 * Prevent Pattern (Jimmy Koppel: Define dangling pointers out of existence)
 * Macro avoids strict-aliasing UB of `(void **)&typed_ptr` while guaranteeing
 * every freed pointer is immediately reset to nullptr at the call site.
 */
#define SAFE_FREE(ptr)                                                         \
  do {                                                                         \
    free(ptr);                                                                 \
    (ptr) = nullptr;                                                           \
  } while (0)

typedef struct {
  int *data;
  size_t len;
  size_t cap;
} IntVec;

typedef struct {
  const int *items;
  size_t len;
} VecView;

typedef struct {
  char *first;
  char *second;
} Pair;

typedef struct ListNode {
  char *label;
  struct ListNode *next;
} ListNode;

static char *local_strdup(const char *s) {
  if (s == nullptr) {
    return nullptr;
  }
  size_t n = strlen(s) + 1;
  char *copy = malloc(n);
  if (copy != nullptr) {
    memcpy(copy, s, n);
  }
  return copy;
}

/*
 * ============================================================================
 * PART 1: IDENTIFY — DELIBERATE DEFECT TRIGGERS
 * Run with --trigger-* under ASan or `leaks --atExit --` / Valgrind to observe
 * each failure mode before implementing the fixes in Part 2.
 * ============================================================================
 */

static void trigger_dangling_uaf(void) {
  printf("[TRIGGER] Dangling Pointer -> Heap Use-After-Free...\n");
  int *p = malloc(sizeof(int));
  assert(p != nullptr);
  *p = 123;
  free(p);
  /* BUG: p is not nulled and is dereferenced after free */
  printf("Stale read: %d\n", *p);
}

static void trigger_dangling_realloc(void) {
  printf("[TRIGGER] Dangling Alias after realloc() relocation...\n");
  int *vec = malloc(2 * sizeof(int));
  assert(vec != nullptr);
  vec[0] = 10;
  vec[1] = 20;

  /* Hold an interior pointer before growing */
  int *alias = &vec[0];

  /* Blocker allocation encourages realloc to relocate vec to a new block */
  void *blocker = malloc(128);
  assert(blocker != nullptr);

  int *grown = realloc(vec, 4096 * sizeof(int));
  assert(grown != nullptr);
  vec = grown;

  /* BUG: if realloc moved the block, `alias` still points to freed memory */
  printf("Alias value after realloc: %d\n", *alias);

  free(blocker);
  free(vec);
}

static void trigger_dangling_alias(void) {
  printf("[TRIGGER] Shallow Struct Copy -> Shared Ownership Double-Free...\n");
  Pair a = {.first = local_strdup("alpha"), .second = local_strdup("beta")};
  assert(a.first != nullptr && a.second != nullptr);

  /* BUG: shallow copy aliases the exact same heap pointers */
  Pair b = a;

  free(a.first);
  free(a.second);
  /* BUG: freeing b frees already-freed pointers (double free!) */
  free(b.first);
  free(b.second);
}

static void trigger_leak_overwrite(void) {
  printf("[TRIGGER] Overwriting Owning Pointer -> Direct Memory Leak...\n");
  char *buf = malloc(32);
  assert(buf != nullptr);
  memcpy(buf, "first", 6);

  /* BUG: original 32-byte block is orphaned without free(buf) */
  buf = malloc(64);
  assert(buf != nullptr);
  memcpy(buf, "second", 7);
  printf("Active buffer: %s\n", buf);
  free(buf);
}

static char *early_return_helper(bool fail_second) {
  char *first = local_strdup("resource_one");
  assert(first != nullptr);

  if (fail_second) {
    /* BUG: forgot to free `first` on error exit path */
    return nullptr;
  }
  return first;
}

static void trigger_leak_early_return(void) {
  printf("[TRIGGER] Partial Initialization Early Return -> Direct Leak...\n");
  char *res = early_return_helper(true);
  /* Scrub stack/registers via a normal call so macOS leaks sees it as unreachable */
  printf("early_return_helper returned %p (leaked `first`)\n", (void *)res);
}

static void trigger_leak_indirect(void) {
  printf("[TRIGGER] Shallow Struct Free -> Indirect Memory Leak...\n");
  Pair *p = malloc(sizeof(Pair));
  assert(p != nullptr);
  p->first = local_strdup("outer_ok");
  p->second = local_strdup("inner_leaked");
  assert(p->first != nullptr && p->second != nullptr);

  /* BUG: freeing p without freeing p->first and p->second orphans inner blocks */
  free(p);
}

/*
 * ============================================================================
 * PART 2: FIX — REPAIR THE BROKEN PATTERNS (IMPLEMENT DRILLS 1–5)
 * ============================================================================
 */

/*
 * Drill 1: Fix Dangling Stack Reference
 *
 * Broken code being replaced:
 *   char *buggy_format_tag(const char *prefix, int id) {
 *     char buf[64];
 *     snprintf(buf, sizeof(buf), "%s-%d", prefix, id);
 *     return buf; // BUG: returns pointer to popped stack frame!
 *   }
 *
 * Requirements:
 * - Return nullptr if prefix == nullptr.
 * - Compute exact required byte length with snprintf(nullptr, 0, "%s-%d", prefix, id).
 * - Allocate (size_t)needed + 1 bytes on the heap via malloc.
 * - Return nullptr if malloc fails.
 * - Format into the heap buffer and return it (caller owns and frees it).
 */
static char *fixed_format_tag(const char *prefix, int id) {
  (void)prefix;
  (void)id;
  return nullptr; /* TODO: implement */
}

/*
 * Drill 2: Fix Dangling Alias Across realloc()
 *
 * Broken code being replaced:
 *   int *tracked = &vec->data[tracked_idx];
 *   vec->data = realloc(vec->data, new_cap * sizeof(int)); // BUG 1: leaks on NULL
 *   // BUG 2: `tracked` still points into old freed block if realloc moved it!
 *
 * Requirements:
 * - Return false if vec == nullptr.
 * - If tracked_out != nullptr:
 *     Verify vec->data != nullptr && tracked_idx < vec->len; otherwise return false.
 * - If vec->len == vec->cap:
 *     Compute new_cap = (vec->cap == 0) ? 2 : (vec->cap > SIZE_MAX / 2 ? 0 : vec->cap * 2);
 *     Guard overflow: new_cap == 0 || new_cap > SIZE_MAX / sizeof(int) -> return false.
 *     Use temporary pointer `int *tmp = realloc(vec->data, new_cap * sizeof(int));`
 *     If tmp == nullptr, leave vec intact and return false.
 *     Assign vec->data = tmp; vec->cap = new_cap;
 * - Append vec->data[vec->len++] = value;
 * - If tracked_out != nullptr, rebase *tracked_out = vec->data + tracked_idx;
 * - Return true.
 */
static bool fixed_vec_push_keep_index(IntVec *vec, int value, size_t tracked_idx,
                                      const int **tracked_out) {
  (void)vec;
  (void)value;
  (void)tracked_idx;
  (void)tracked_out;
  return false; /* TODO: implement */
}

/*
 * Drill 3: Fix Partial-Init Leak & Indirect Leak
 *
 * Broken code being replaced:
 *   Pair *p = malloc(sizeof(Pair));
 *   p->first = local_strdup(first);
 *   if (fail_second_for_test) return nullptr; // BUG: leaks p and p->first!
 *
 * Requirements for fixed_pair_create:
 * - Return nullptr if first == nullptr || second == nullptr.
 * - Allocate Pair *p; check for nullptr.
 * - Initialize p->first = nullptr; p->second = nullptr; BEFORE sub-allocations.
 * - Allocate p->first = local_strdup(first); on failure jump to cleanup.
 * - If fail_second_for_test is true, simulate allocation failure for p->second
 *   and jump to cleanup; otherwise allocate p->second = local_strdup(second).
 * - Cleanup unwinds and frees p->second, p->first, and p before returning nullptr.
 *
 * Requirements for fixed_pair_destroy:
 * - If pp == nullptr || *pp == nullptr, return.
 * - Free (*pp)->first, (*pp)->second, and *pp (in inner-to-outer order).
 * - Set *pp = nullptr (defines dangling pointers & double-frees out of existence).
 */
static Pair *fixed_pair_create(const char *first, const char *second,
                               bool fail_second_for_test) {
  (void)first;
  (void)second;
  (void)fail_second_for_test;
  return nullptr; /* TODO: implement */
}

static void fixed_pair_destroy(Pair **pp) {
  (void)pp;
  /* TODO: implement */
}

/*
 * Drill 4: Fix Pointer-Overwrite Leak
 *
 * Broken code being replaced:
 *   *slot = local_strdup(new_val); // BUG: leaks previous *slot!
 *
 * Requirements:
 * - Return false if slot == nullptr || new_val == nullptr.
 * - If fail_alloc_for_test is true (or local_strdup returns nullptr):
 *     Leave *slot completely unchanged (do NOT free or overwrite it) and return false.
 * - Allocate copy = local_strdup(new_val) first.
 * - Once copy succeeds, free(*slot) and assign *slot = copy.
 * - Return true.
 */
static bool fixed_replace_string(char **slot, const char *new_val,
                                 bool fail_alloc_for_test) {
  (void)slot;
  (void)new_val;
  (void)fail_alloc_for_test;
  return false; /* TODO: implement */
}

/*
 * Drill 5: Fix Linked List Traversal Use-After-Free
 *
 * Broken code being replaced:
 *   for (ListNode *c = *head; c != nullptr; c = c->next) {
 *     free(c->label);
 *     free(c); // BUG: loop step reads c->next AFTER free(c)!
 *   }
 *
 * Requirements:
 * - If head == nullptr, return.
 * - Walk the list saving `ListNode *next = curr->next;` BEFORE freeing `curr->label`
 *   and `curr`.
 * - Set *head = nullptr before returning.
 */
static void fixed_node_list_free(ListNode **head) {
  (void)head;
  /* TODO: implement */
}

/*
 * ============================================================================
 * PART 3: PREVENT — OWNERSHIP INVARIANTS BY DESIGN (IMPLEMENT DRILLS 6–7)
 * ============================================================================
 */

/*
 * Drill 6: Ownership Transfer (Move Semantics)
 *
 * Prevents shallow-copy double-frees and dangling aliases by giving every heap
 * buffer a single exclusive owner. Moving transfers ownership and zeroes src.
 *
 * Requirements:
 * - Return false if dst == nullptr || src == nullptr.
 * - If dst == src, nothing to move; return true (must NOT free itself!).
 * - Free any existing storage owned by dst (`SAFE_FREE(dst->data)`) to prevent
 *   an overwrite leak.
 * - Transfer ownership: *dst = *src;
 * - Reset src to empty inert state: src->data = nullptr; src->len = 0; src->cap = 0;
 * - Return true.
 */
static bool owned_vec_move(IntVec *dst, IntVec *src) {
  (void)dst;
  (void)src;
  return false; /* TODO: implement */
}

/*
 * Drill 7: Borrowed Non-Owning View vs Owning Buffer (Daniel Jackson Concept Design)
 *
 * Separates "storage ownership" (IntVec) from "read-only slice inspection" (VecView).
 * VecView never allocates or frees; it is created fresh after mutations.
 *
 * Requirements for owned_vec_view:
 * - If vec == nullptr || vec->data == nullptr || start > end || end > vec->len:
 *     Return (VecView){.items = nullptr, .len = 0}.
 * - Return (VecView){.items = vec->data + start, .len = end - start}.
 *
 * Requirements for vec_view_get:
 * - Return false if view.items == nullptr || out == nullptr || idx >= view.len.
 * - Assign *out = view.items[idx] and return true.
 */
static VecView owned_vec_view(const IntVec *vec, size_t start, size_t end) {
  (void)vec;
  (void)start;
  (void)end;
  return (VecView){.items = nullptr, .len = 0}; /* TODO: implement */
}

static bool vec_view_get(VecView view, size_t idx, int *out) {
  (void)view;
  (void)idx;
  (void)out;
  return false; /* TODO: implement */
}

/*
 * ============================================================================
 * TEST SUITE
 * ============================================================================
 */

static void test_drill1_format_tag(void) {
  assert(fixed_format_tag(nullptr, 1) == nullptr);
  char *tag = fixed_format_tag("worker", 42);
  assert(tag != nullptr);
  assert(strcmp(tag, "worker-42") == 0);
  SAFE_FREE(tag);
  assert(tag == nullptr);
}

static void test_drill2_push_and_rebase(void) {
  IntVec vec = {0};
  assert(fixed_vec_push_keep_index(&vec, 11, 0, nullptr) == true);
  assert(fixed_vec_push_keep_index(&vec, 22, 0, nullptr) == true);

  const int *tracked = nullptr;
  for (int i = 0; i < 500; i++) {
    assert(fixed_vec_push_keep_index(&vec, 100 + i, 1, &tracked) == true);
    assert(tracked == vec.data + 1);
    assert(*tracked == 22);
  }
  assert(vec.len == 502);
  SAFE_FREE(vec.data);
  vec.len = 0;
  vec.cap = 0;
}

static void test_drill3_pair_lifecycle(void) {
  assert(fixed_pair_create(nullptr, "b", false) == nullptr);
  assert(fixed_pair_create("a", nullptr, false) == nullptr);

  /* Simulated mid-init failure must free first string + Pair with 0 leaks */
  assert(fixed_pair_create("first_ok", "second_fail", true) == nullptr);

  Pair *p = fixed_pair_create("key", "value", false);
  assert(p != nullptr);
  assert(strcmp(p->first, "key") == 0);
  assert(strcmp(p->second, "value") == 0);

  fixed_pair_destroy(&p);
  assert(p == nullptr);
  /* Idempotent second destroy must be safe */
  fixed_pair_destroy(&p);
}

static void test_drill4_replace_string(void) {
  char *slot = local_strdup("initial");
  assert(slot != nullptr);

  /* Failed replacement preserves existing string without leaking or freeing */
  assert(fixed_replace_string(&slot, "ignored", true) == false);
  assert(strcmp(slot, "initial") == 0);

  /* Successful replacement frees "initial" and stores "updated" */
  assert(fixed_replace_string(&slot, "updated", false) == true);
  assert(strcmp(slot, "updated") == 0);

  assert(fixed_replace_string(nullptr, "x", false) == false);
  SAFE_FREE(slot);
}

static void test_drill5_node_list_free(void) {
  ListNode *n3 = malloc(sizeof(ListNode));
  ListNode *n2 = malloc(sizeof(ListNode));
  ListNode *n1 = malloc(sizeof(ListNode));
  assert(n1 != nullptr && n2 != nullptr && n3 != nullptr);

  n1->label = local_strdup("one");
  n1->next = n2;
  n2->label = local_strdup("two");
  n2->next = n3;
  n3->label = local_strdup("three");
  n3->next = nullptr;
  assert(n1->label != nullptr && n2->label != nullptr && n3->label != nullptr);

  fixed_node_list_free(&n1);
  assert(n1 == nullptr);
  fixed_node_list_free(&n1);
}

static void test_drill6_move_semantics(void) {
  IntVec src = {0};
  IntVec dst = {0};
  assert(fixed_vec_push_keep_index(&src, 10, 0, nullptr) == true);
  assert(fixed_vec_push_keep_index(&src, 20, 0, nullptr) == true);
  assert(fixed_vec_push_keep_index(&dst, 99, 0, nullptr) == true);

  int *expected_ptr = src.data;
  assert(owned_vec_move(&dst, &src) == true);
  assert(dst.data == expected_ptr && dst.len == 2);
  assert(dst.data[0] == 10 && dst.data[1] == 20);
  assert(src.data == nullptr && src.len == 0 && src.cap == 0);

  /* Self-move must not destroy dst */
  assert(owned_vec_move(&dst, &dst) == true);
  assert(dst.data == expected_ptr && dst.len == 2);

  SAFE_FREE(dst.data);
  SAFE_FREE(src.data);
}

static void test_drill7_borrowed_view(void) {
  IntVec vec = {0};
  for (int i = 0; i < 5; i++) {
    assert(fixed_vec_push_keep_index(&vec, i * 10, 0, nullptr) == true);
  }

  VecView bad = owned_vec_view(&vec, 3, 2);
  assert(bad.items == nullptr && bad.len == 0);
  VecView oob = owned_vec_view(&vec, 1, 6);
  assert(oob.items == nullptr && oob.len == 0);

  VecView view = owned_vec_view(&vec, 1, 4);
  assert(view.items == vec.data + 1 && view.len == 3);

  int val = -1;
  assert(vec_view_get(view, 0, &val) == true && val == 10);
  assert(vec_view_get(view, 2, &val) == true && val == 30);
  assert(vec_view_get(view, 3, &val) == false);
  assert(vec_view_get(view, 0, nullptr) == false);

  SAFE_FREE(vec.data);
}

int main(int argc, char *argv[]) {
  if (argc > 1) {
    if (strcmp(argv[1], "--trigger-dangling-uaf") == 0) {
      trigger_dangling_uaf();
      return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "--trigger-dangling-realloc") == 0) {
      trigger_dangling_realloc();
      return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "--trigger-dangling-alias") == 0) {
      trigger_dangling_alias();
      return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "--trigger-leak-overwrite") == 0) {
      trigger_leak_overwrite();
      return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "--trigger-leak-early-return") == 0) {
      trigger_leak_early_return();
      return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "--trigger-leak-indirect") == 0) {
      trigger_leak_indirect();
      return EXIT_SUCCESS;
    }
  }

  test_drill1_format_tag();
  test_drill2_push_and_rebase();
  test_drill3_pair_lifecycle();
  test_drill4_replace_string();
  test_drill5_node_list_free();
  test_drill6_move_semantics();
  test_drill7_borrowed_view();

  printf("[PASS] Exercise 52: Dangling References & Memory Leaks verified cleanly.\n");
  return EXIT_SUCCESS;
}
