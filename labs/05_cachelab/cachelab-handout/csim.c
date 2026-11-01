#include "cachelab.h"
#include <getopt.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void usage(const char *name) {
  printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n"
         "Options:\n"
         "  -h         Print this help message.\n"
         "  -v         Optional verbose flag.\n"
         "  -s <num>   Number of set index bits.\n"
         "  -E <num>   Number of lines per set.\n"
         "  -b <num>   Number of block offset bits.\n"
         "  -t <file>  Trace file.\n"
         "\n"
         "Examples:\n"
         "  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n"
         "  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n",
         name, name, name);
}

typedef struct {
  int s;
  int E;
  int b;
  char *t;
  bool v;
} args_t;

args_t parse_args(int argc, char *argv[]) {
  args_t args;
  args.s = args.E = args.b = -1;
  args.t = nullptr;
  args.v = false;

  int opt;
  while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
    switch (opt) {
    case 'h':
      usage(argv[0]);
      exit(EXIT_SUCCESS);
    case 's':
      args.s = atoi(optarg);
      break;
    case 'E':
      args.E = atoi(optarg);
      break;
    case 'b':
      args.b = atoi(optarg);
      break;
    case 't':
      args.t = optarg;
      break;
    case 'v':
      args.v = true;
      break;
    default:
      usage(argv[0]);
      exit(EXIT_FAILURE);
    }
  }
  if (args.s < 0 || args.E <= 0 || args.b < 0 || args.t == nullptr) {
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }
  return args;
}

/*
 * TODO: Define your cache line, cache set, and cache metadata structures.
 *
 * Architectural Guidance:
 * - line_t: Needs valid flag (bool), tag (uint64_t), and an LRU
 * counter/timestamp (e.g. int clk).
 * - set_t: Owns an allocated array of E line_t elements.
 * - cache_t: Owns an allocated array of S = 2^s set_t elements, plus s, E, b
 * geometry parameters.
 */

typedef struct {
  bool valid;
  uint64_t tag;
  int clk;
} line_t;

typedef struct {
  line_t *lines;
} set_t;

typedef struct {
  set_t *sets;
  int s;
  int E;
  int b;
} cache_t;

/* TODO: Define cache event constants */
[[maybe_unused]] constexpr int CACHE_HIT = 0;
[[maybe_unused]] constexpr int CACHE_MISS = 1;
[[maybe_unused]] constexpr int CACHE_EVICT = 2;

/*
 * TODO: Implement cache_init:
 * Allocate memory for S = 1 << s sets and E lines per set.
 * Initialize valid bits to false.
 */
void cache_init(cache_t *cache, int s, int E, int b) {
  cache->s = s;   // number of set index bits
  cache->E = E;   // number of lines per set, the cache associativity
  cache->b = b;   // number of block offset bits
  int S = 1 << s; // number of cache sets 2^s
  cache->sets = malloc(S * sizeof(set_t));
  if (cache->sets == nullptr) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  // Allocate array E line_t structures for set i
  for (int i = 0; i < S; i++) {
    cache->sets[i].lines = malloc(E * sizeof(line_t));
    if (cache->sets[i].lines == nullptr) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }
    //
    for (int j = 0; j < E; j++) {
      cache->sets[i].lines[j].valid = false;
      cache->sets[i].lines[j].tag = 0;
      cache->sets[i].lines[j].clk = 0;
    }
  }
  // free(cache->sets[i].lines);
}

/*
 * Test contract (tests/test_csim.c calls these exact names and signatures):
 *   line_t  { bool valid; uint64_t tag; int clk; }
 *   set_t   { line_t *lines; }
 *   cache_t { int s; int E; int b; set_t *sets; }
 *   void cache_init(cache_t *cache, int s, int E, int b);
 *   void cache_destroy(cache_t *cache);
 *   int  cache_access(cache_t *cache, uint64_t addr, int clk);
 */

/*
 * TODO: Implement cache_destroy:
 * Free all dynamically allocated line arrays and the set array to ensure zero
 * Valgrind leaks.
 */

/*
 * TODO: Implement cache_access:
 * Extract set index = (addr >> b) & ((1ULL << s) - 1)
 * Extract tag = addr >> (s + b)
 * Check for hit (valid && line.tag == tag)
 * Check for cold miss (first line with !valid)
 * If full, find LRU line (smallest timestamp) and evict
 * Returns CACHE_HIT, CACHE_MISS, or CACHE_MISS | CACHE_EVICT.
 */

int main(int argc, char *argv[]) {
  args_t args = parse_args(argc, argv);

  FILE *fp = fopen(args.t, "r");
  if (fp == nullptr) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  int hits = 0;
  int misses = 0;
  int evictions = 0;

  /*
   * TODO: Initialize your cache structure here using args.s, args.E, args.b.
   */

  char line[32];
  while (fgets(line, sizeof(line), fp) != nullptr) {
    [[maybe_unused]] char type;
    [[maybe_unused]] uint64_t addr;
    [[maybe_unused]] int size;

    /* Ignore instruction loads (lines without leading space or type 'I') */
    if (sscanf(line, " %c %" SCNx64 ",%d", &type, &addr, &size) != 3) {
      continue;
    }
    if (type != 'L' && type != 'S' && type != 'M') {
      continue;
    }

    /*
     * TODO: Simulate the memory reference(s):
     * - 'L' (Load): 1 cache access
     * - 'S' (Store): 1 cache access
     * - 'M' (Modify): 2 cache accesses (1 Load followed by 1 Store)
     *
     * Increment hits, misses, and evictions based on the access result.
     * If args.v is true, format and print trace outputs.
     */
  }

  /*
   * TODO: Destroy and free all cache memory allocations.
   */

  if (fclose(fp) != 0) {
    perror("fclose");
    exit(EXIT_FAILURE);
  }

  /* Autograder entry point: prints summary to stdout and writes .csim_results
   */
  printSummary(hits, misses, evictions);
  return 0;
}
