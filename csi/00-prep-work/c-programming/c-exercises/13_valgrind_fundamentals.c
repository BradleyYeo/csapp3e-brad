/*
 * Exercise 13: Valgrind Fundamentals and Memory Diagnostics
 *
 * Compile and run natively:
 *   clang -std=c23 -Wall -Wextra -Werror -pedantic -g -o 13_valgrind_fundamentals 13_valgrind_fundamentals.c && ./13_valgrind_fundamentals
 *
 * Run under Valgrind (Linux / WSL / Docker):
 *   valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./13_valgrind_fundamentals
 */
#define _POSIX_C_SOURCE 200809L
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
#include <stdbool.h>
#include <stdalign.h>
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
#include <unistd.h>

#if defined(__has_include)
#if __has_include(<valgrind/memcheck.h>)
#include <valgrind/memcheck.h>
#define HAVE_VALGRIND_CLIENT_REQUESTS 1
#endif
#endif

#ifndef HAVE_VALGRIND_CLIENT_REQUESTS
#define VALGRIND_MAKE_MEM_NOACCESS(addr, len)   ((void)0)
#define VALGRIND_MAKE_MEM_UNDEFINED(addr, len)  ((void)0)
#define VALGRIND_MAKE_MEM_DEFINED(addr, len)    ((void)0)
#define HAVE_VALGRIND_CLIENT_REQUESTS 0
#endif

/*
 * Concept Design (Daniel Jackson):
 * 1. SymbolEntry: Concrete dynamic record holding key/value payloads and metadata.
 * 2. ScopeEnvironment: Owns a lexical scope level and manages lifetime of symbols.
 * 3. ScopeCursor: Borrowed read-only cursor for lookup operations (non-owning).
 */

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

/*
 * Drill 1: Fully Initialized Entry Constructor
 *
 * Requirements:
 * - Return nullptr if key or val is nullptr.
 * - Allocate a SymbolEntry structure.
 * - Duplicate key and val onto the heap (ensure null termination).
 * - Initialize val_len to strlen(val).
 * - Initialize symbol_id and is_constant.
 * - Initialize next to nullptr (Jimmy Koppel: define dangling pointers out of existence).
 * - Crucial Valgrind Rule: Every field must be initialized explicitly.
 *   Leaving any struct member uninitialized marks its V-bits (validity bits) as 0,
 *   which will trigger uninitialized value diagnostics when read.
 */
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

/*
 * Drill 2: Scope Environment Creation and Insertion
 *
 * Requirements:
 * - scope_env_create: Allocate ScopeEnvironment, set scope_level, parent, head=nullptr, symbol_count=0.
 * - scope_env_insert: Prepend entry to env->head, increment symbol_count, return true.
 * - Defensive check: Reject nullptr env or entry.
 */
static ScopeEnvironment *scope_env_create(int scope_level, ScopeEnvironment *parent) {
  ScopeEnvironment *env = malloc(sizeof(ScopeEnvironment));
  if (env == nullptr) {
    return nullptr;
  }
  env->scope_level = scope_level;
  env->symbol_count = 0;
  env->head = nullptr;
  env->parent = parent;
  return env;
}

static bool scope_env_insert(ScopeEnvironment *env, SymbolEntry *entry) {
  if (env == nullptr || entry == nullptr) {
    return false;
  }
  entry->next = env->head;
  env->head = entry;
  env->symbol_count++;
  return true;
}

/*
 * Drill 3: Recursive / Cascading Scope Deallocation
 *
 * Requirements:
 * - Traverse the linked list of SymbolEntry nodes.
 * - Free entry->key and entry->val.
 * - Free entry itself.
 * - Reset env->head to nullptr and env->symbol_count to 0.
 * - Free env itself.
 * - Mental Model (Valgrind Leak Classification):
 *   If the root env pointer is dropped without traversing env->head,
 *   env is reported as "definitely lost", and all SymbolEntry nodes + strings
 *   are reported as "indirectly lost".
 */
static void scope_env_destroy(ScopeEnvironment *env) {
  if (env == nullptr) {
    return;
  }

  SymbolEntry *curr = env->head;
  while (curr != nullptr) {
    SymbolEntry *next = curr->next;
    free(curr->key);
    curr->key = nullptr;
    free(curr->val);
    curr->val = nullptr;
    free(curr);
    curr = next;
  }

  env->head = nullptr;
  env->symbol_count = 0;
  free(env);
}

/*
 * Drill 4: Overlapping Memory Safety (memmove vs memcpy)
 *
 * In standard C, calling memcpy() on overlapping memory ranges is undefined behavior.
 * Valgrind Memcheck actively detects this with:
 *   "Source and destination overlap in memcpy(...)"
 *
 * Requirements:
 * - Safely shift a substring within entry->val in place.
 * - Must use memmove() to handle overlapping regions correctly.
 */
static bool symbol_entry_shift_value(SymbolEntry *entry, size_t dest_offset, size_t src_offset, size_t count) {
  if (entry == nullptr || entry->val == nullptr) {
    return false;
  }
  if (src_offset + count > entry->val_len || dest_offset + count > entry->val_len) {
    return false;
  }

  /* Safe for overlapping source and destination */
  memmove(entry->val + dest_offset, entry->val + src_offset, count);
  return true;
}

/*
 * Lookup Helper (Non-owning borrowed cursor)
 */
static const SymbolEntry *scope_env_lookup(const ScopeEnvironment *env, const char *key) {
  if (env == nullptr || key == nullptr) {
    return nullptr;
  }

  for (const ScopeEnvironment *scope = env; scope != nullptr; scope = scope->parent) {
    for (const SymbolEntry *entry = scope->head; entry != nullptr; entry = entry->next) {
      if (strcmp(entry->key, key) == 0) {
        return entry;
      }
    }
  }
  return nullptr;
}

/*
 * ============================================================================
 * Deliberate Defect Triggers for Valgrind Memcheck Diagnostic Study
 * ============================================================================
 */

/* 1. Uninitialized Variable Used in Conditional Branch */
static void trigger_uninit_branch(void) {
  printf("[TRIGGER] Branching on uninitialized variable...\n");
  int *uninitialized_box = malloc(sizeof(int));
  assert(uninitialized_box != nullptr);

  /*
   * Valgrind lazy reporting: copying or doing arithmetic on uninitialized memory
   * does NOT immediately trigger an error. The error triggers here because
   * the program flow conditionally branches based on undefined bits (V-bits = 0).
   * With --track-origins=yes, Valgrind points directly to the malloc call above.
   */
  if (*uninitialized_box > 0) {
    printf("Branch taken (box = %d)\n", *uninitialized_box);
  } else {
    printf("Branch not taken (box = %d)\n", *uninitialized_box);
  }
  free(uninitialized_box);
}

/* 2. Uninitialized Memory Passed to System Call */
static void trigger_uninit_syscall(void) {
  printf("[TRIGGER] Passing uninitialized buffer to write(2) syscall...\n");
  char *buffer = malloc(32); /* Allocated, A-bits=1, V-bits=0 */
  assert(buffer != nullptr);

  /*
   * Syscall arguments that read undefined memory trigger:
   * "Syscall param write(buf) points to uninitialised byte(s)"
   */
  ssize_t written = write(STDOUT_FILENO, buffer, 16);
  (void)written;
  printf("\n");
  free(buffer);
}

/* 3. Heap Out-of-Bounds Read */
static void trigger_invalid_read(void) {
  printf("[TRIGGER] Reading past allocated heap block...\n");
  int *array = malloc(4 * sizeof(int));
  assert(array != nullptr);
  for (int i = 0; i < 4; i++) {
    array[i] = i * 10;
  }

  /*
   * Index 4 is out of bounds (elements are 0..3).
   * Triggers: "Invalid read of size 4"
   */
  int value = array[4];
  printf("Read past bounds value: %d\n", value);
  free(array);
}

/* 4. Heap Out-of-Bounds Write */
static void trigger_invalid_write(void) {
  printf("[TRIGGER] Writing past allocated heap block...\n");
  uint8_t *buf = malloc(8);
  assert(buf != nullptr);

  /*
   * Offset 8 is past the 8-byte allocation [0..7].
   * Triggers: "Invalid write of size 1"
   */
  buf[8] = 0xAA;
  printf("Corrupted byte written.\n");
  free(buf);
}

/* 5. Memory Leak: Definitely Lost */
static void trigger_definitely_lost(void) {
  printf("[TRIGGER] Generating 'definitely lost' memory leak...\n");
  SymbolEntry *entry = symbol_entry_create("API_KEY", "secret_12345", 101, 1);
  assert(entry != nullptr);

  /*
   * Dropping the pointer completely with no remaining references.
   * Valgrind: "definitely lost: X bytes in 1 blocks"
   */
  entry = nullptr;
  (void)entry;
}

/* 6. Memory Leak: Indirectly Lost */
static void trigger_indirectly_lost(void) {
  printf("[TRIGGER] Generating 'indirectly lost' memory leak (linked chain)...\n");
  ScopeEnvironment *env = scope_env_create(1, nullptr);
  assert(env != nullptr);

  SymbolEntry *e1 = symbol_entry_create("node_a", "val_a", 1, 0);
  SymbolEntry *e2 = symbol_entry_create("node_b", "val_b", 2, 0);
  SymbolEntry *e3 = symbol_entry_create("node_c", "val_c", 3, 0);

  scope_env_insert(env, e1);
  scope_env_insert(env, e2);
  scope_env_insert(env, e3);

  /*
   * Dropping env pointer without destroying children.
   * Valgrind:
   * - env is reported as "definitely lost"
   * - e1, e2, e3, keys, and values are reported as "indirectly lost"
   *   because they are reachable only through the definitely lost env block!
   */
  env = nullptr;
  (void)env;
}

/* 7. Memory Leak: Possibly Lost (Interior Pointer) */
static void *global_interior_ptr = nullptr;

static void trigger_possibly_lost(void) {
  printf("[TRIGGER] Generating 'possibly lost' leak via interior pointer...\n");
  char *raw_block = malloc(128);
  assert(raw_block != nullptr);

  /*
   * Storing a pointer into the interior (+32 bytes), then dropping base pointer.
   * Valgrind: "possibly lost: 128 bytes in 1 blocks"
   */
  global_interior_ptr = raw_block + 32;
  printf("Interior pointer retained at %p\n", global_interior_ptr);
  raw_block = nullptr;
  (void)raw_block;
}

/* 8. Overlapping Memory in memcpy */
static void trigger_overlap(void) {
  printf("[TRIGGER] Invoking memcpy on overlapping memory ranges...\n");
  char text[32];
  memcpy(text, "abcdefghijklmnop", 17);

  /*
   * Source: text (bytes 0..10)
   * Destination: text + 2 (bytes 2..12)
   * Range [2..10] overlaps!
   * Valgrind: "Source and destination overlap in memcpy(...)"
   */
  memcpy(text + 2, text, 10);
  printf("Result text: %s\n", text);
}

/* 9. Valgrind Client Request Custom Arena Poisoning */
static void trigger_client_poison(void) {
  printf("[TRIGGER] Testing custom allocator memory poisoning...\n");
  uint8_t *custom_arena = malloc(64);
  assert(custom_arena != nullptr);

  /* Poison the arena using Valgrind client request */
  VALGRIND_MAKE_MEM_NOACCESS(custom_arena, 64);

  printf("Accessing poisoned custom memory...\n");
  /* If running under Valgrind with headers, this triggers Invalid Read */
  uint8_t byte = custom_arena[0];
  printf("Byte read: 0x%02x\n", byte);

  VALGRIND_MAKE_MEM_DEFINED(custom_arena, 64);
  free(custom_arena);
}

int main(int argc, char *argv[]) {
  if (argc > 1) {
    if (strcmp(argv[1], "--trigger-uninit-branch") == 0) {
      trigger_uninit_branch();
      return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "--trigger-uninit-syscall") == 0) {
      trigger_uninit_syscall();
      return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "--trigger-invalid-read") == 0) {
      trigger_invalid_read();
      return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "--trigger-invalid-write") == 0) {
      trigger_invalid_write();
      return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "--trigger-definitely-lost") == 0) {
      trigger_definitely_lost();
      return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "--trigger-indirectly-lost") == 0) {
      trigger_indirectly_lost();
      return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "--trigger-possibly-lost") == 0) {
      trigger_possibly_lost();
      return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "--trigger-overlap") == 0) {
      trigger_overlap();
      return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "--trigger-client-poison") == 0) {
      trigger_client_poison();
      return EXIT_SUCCESS;
    }
  }

  /*
   * Default Execution Mode: Full unit test verification of clean memory lifecycle.
   * Running this under Valgrind should report:
   *   "ERROR SUMMARY: 0 errors from 0 contexts"
   *   "All heap blocks were freed -- no leaks are possible"
   */

  /* Test 1: Scope hierarchy and symbol insertion */
  ScopeEnvironment *global_scope = scope_env_create(0, nullptr);
  assert(global_scope != nullptr);
  assert(global_scope->scope_level == 0 && global_scope->symbol_count == 0);

  SymbolEntry *s1 = symbol_entry_create("PORT", "8080", 1, 1);
  SymbolEntry *s2 = symbol_entry_create("HOST", "127.0.0.1", 2, 1);
  assert(s1 != nullptr && s2 != nullptr);
  assert(scope_env_insert(global_scope, s1) == true);
  assert(scope_env_insert(global_scope, s2) == true);
  assert(global_scope->symbol_count == 2);

  /* Test 2: Nested local scope */
  ScopeEnvironment *local_scope = scope_env_create(1, global_scope);
  assert(local_scope != nullptr);
  assert(local_scope->parent == global_scope);

  SymbolEntry *s3 = symbol_entry_create("TIMEOUT", "30", 3, 0);
  assert(s3 != nullptr);
  assert(scope_env_insert(local_scope, s3) == true);

  /* Test 3: Lexical resolution across scope chain */
  const SymbolEntry *found_local = scope_env_lookup(local_scope, "TIMEOUT");
  assert(found_local != nullptr && strcmp(found_local->val, "30") == 0);

  const SymbolEntry *found_global = scope_env_lookup(local_scope, "PORT");
  assert(found_global != nullptr && strcmp(found_global->val, "8080") == 0);

  const SymbolEntry *not_found = scope_env_lookup(local_scope, "MISSING");
  assert(not_found == nullptr);

  /* Test 4: Overlapping value shift using memmove */
  SymbolEntry *mut_entry = symbol_entry_create("BUFFER", "PREFIX_DATA_SUFFIX", 4, 0);
  assert(mut_entry != nullptr);
  /* Shift "DATA" (4 bytes from index 7) to index 0 */
  assert(symbol_entry_shift_value(mut_entry, 0, 7, 4) == true);
  assert(strncmp(mut_entry->val, "DATA", 4) == 0);

  /* Reclaim mut_entry */
  free(mut_entry->key);
  free(mut_entry->val);
  free(mut_entry);

  /* Test 5: Clean cascaded destruction */
  scope_env_destroy(local_scope);
  scope_env_destroy(global_scope);

  printf("[PASS] Exercise 13: Valgrind Fundamentals verified cleanly.\n");
  return EXIT_SUCCESS;
}
