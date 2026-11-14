/* K&R 6-2: Group variable names that are identical in the first k characters
 * (default 6) but different somewhere thereafter, ordered alphabetically.
 * Ignore words inside string literals and comments.
 */
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

struct word_node {
  char *word;
  struct word_node *next; /* sorted list of distinct words sharing first k chars */
  struct word_node *left;
  struct word_node *right;
};

/*
 * add_word: inserts `word` into the BST keyed by the first `k` characters of
 * `word`. Words with identical `k`-char prefixes are chained on `next` in
 * alphabetical order (deduplicating exact matches).
 */
struct word_node *add_word(struct word_node *root, const char *word, size_t k) {
  (void)root;
  (void)word;
  (void)k;
  /* TODO: implement */
  return nullptr;
}

/*
 * count_groups: returns the number of BST nodes (distinct k-char prefix
 * groups) in `root`.
 */
size_t count_groups(const struct word_node *root) {
  (void)root;
  /* TODO: implement */
  return 0;
}

/*
 * free_tree: frees all nodes and duplicated strings in `root`.
 */
void free_tree(struct word_node *root) {
  (void)root;
  /* TODO: implement */
}

int main(void) {
  /* Reference getch/ungetch from _resources/getch.c so -Werror stays clean */
  if (0) {
    ungetch(getch());
  }

  struct word_node *root = nullptr;
  root = add_word(root, "counter_b", 6);
  root = add_word(root, "counter_a", 6);
  root = add_word(root, "counter_a", 6); /* duplicate should not add */
  root = add_word(root, "alpha_one", 6);
  root = add_word(root, "zebra_val", 6);

  assert(count_groups(root) == 3);
  assert(root != nullptr && strcmp(root->word, "counter_a") == 0);
  assert(root->next != nullptr && strcmp(root->next->word, "counter_b") == 0);
  assert(root->next->next == nullptr);
  assert(root->left != nullptr && strcmp(root->left->word, "alpha_one") == 0);
  assert(root->right != nullptr && strcmp(root->right->word, "zebra_val") == 0);

  free_tree(root);
  printf("[PASS] Exercise 40: getword groups verified.\n");
  return EXIT_SUCCESS;
}
