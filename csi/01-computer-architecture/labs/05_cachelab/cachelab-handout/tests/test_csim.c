/*
 * test_csim.c - Progressive unit tests for csim.c.
 *
 * Build and run via the Makefile:
 *   make test STAGE=1   # structs + constants
 *   make test STAGE=2   # + cache_init
 *   make test STAGE=3   # + cache_destroy
 *   make test STAGE=4   # + cache_access (decode, hit/miss, eviction, LRU)
 *   make test STAGE=5   # + end-to-end trace tests (tests/run_traces.sh)
 *
 * csim.c is #included directly so the tests can reach its internal
 * functions. Its main() is renamed to csim_main to avoid a clash.
 */
#define main csim_main
#include "../csim.c"
#undef main

#include <stdint.h>
#include <stdio.h>

#ifndef STAGE
#define STAGE 1
#endif

static int checks_run;
static int checks_failed;

#define CHECK(cond)                                                            \
  do {                                                                         \
    checks_run++;                                                              \
    if (!(cond)) {                                                             \
      checks_failed++;                                                         \
      fprintf(stderr, "    FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);      \
    }                                                                          \
  } while (0)

#define CHECK_EQ(actual, expected)                                             \
  do {                                                                         \
    checks_run++;                                                              \
    long long a_ = (long long)(actual);                                        \
    long long e_ = (long long)(expected);                                      \
    if (a_ != e_) {                                                            \
      checks_failed++;                                                         \
      fprintf(stderr, "    FAIL %s:%d: %s == %lld, expected %lld\n",           \
              __FILE__, __LINE__, #actual, a_, e_);                            \
    }                                                                          \
  } while (0)

#define RUN(test)                                                              \
  do {                                                                         \
    int before_ = checks_failed;                                               \
    test();                                                                    \
    printf("  %s %s\n", checks_failed == before_ ? "PASS" : "FAIL", #test);    \
  } while (0)

/* ======================================================================== */
/* Stage 1: struct layout and event constants                               */
/* ======================================================================== */
#if STAGE >= 1

/* Compile-time checks: these fail the build, not the run. */
static_assert(CACHE_HIT == 0, "CACHE_HIT must be 0 so `if (ret)` means miss");
static_assert((CACHE_MISS & CACHE_EVICT) == 0,
              "CACHE_MISS and CACHE_EVICT must be distinct bits");
static_assert(_Generic(((line_t){}).tag, uint64_t: 1, default: 0),
              "line_t.tag must be uint64_t (64-bit addresses)");

static void test_line_fields(void) {
  line_t line = {};
  line.valid = true;
  line.tag = UINT64_MAX; /* would truncate if tag were narrower than 64 bits */
  line.clk = 42;
  CHECK(line.valid);
  CHECK(line.tag == UINT64_MAX);
  CHECK_EQ(line.clk, 42);
}

static void test_set_and_cache_fields(void) {
  line_t lines[2] = {};
  set_t set = {.lines = lines};
  cache_t cache = {.s = 3, .E = 2, .b = 4, .sets = &set};
  CHECK(cache.sets[0].lines == lines);
  CHECK_EQ(cache.s, 3);
  CHECK_EQ(cache.E, 2);
  CHECK_EQ(cache.b, 4);
}

static void stage1(void) {
  puts("Stage 1: structs and constants");
  RUN(test_line_fields);
  RUN(test_set_and_cache_fields);
}
#endif

/* ======================================================================== */
/* Stage 2: cache_init                                                      */
/* ======================================================================== */
#if STAGE >= 2

/* Verifies geometry, allocation sizes, and that every line starts invalid.
 * Writing to every line lets AddressSanitizer catch undersized mallocs. */
static void check_init_shape(int s, int E, int b) {
  cache_t cache;
  cache_init(&cache, s, E, b);

  CHECK_EQ(cache.s, s);
  CHECK_EQ(cache.E, E);
  CHECK_EQ(cache.b, b);
  CHECK(cache.sets != nullptr);

  int S = 1 << s;
  for (int i = 0; i < S; i++) {
    CHECK(cache.sets[i].lines != nullptr);
    for (int j = 0; j < E; j++) {
      CHECK(!cache.sets[i].lines[j].valid);
      cache.sets[i].lines[j].tag = (uint64_t)j; /* touch: ASan bounds check */
    }
  }
  /* Deliberately leaked until Stage 3 provides cache_destroy. */
#if STAGE >= 3
  cache_destroy(&cache);
#endif
}

static void test_init_direct_mapped(void) { check_init_shape(4, 1, 4); }
static void test_init_set_associative(void) { check_init_shape(2, 4, 3); }
static void test_init_fully_associative(void) { check_init_shape(0, 8, 0); }
static void test_init_lab_geometry(void) { check_init_shape(5, 1, 5); }

static void stage2(void) {
  puts("Stage 2: cache_init");
  RUN(test_init_direct_mapped);
  RUN(test_init_set_associative);
  RUN(test_init_fully_associative);
  RUN(test_init_lab_geometry);
}
#endif

/* ======================================================================== */
/* Stage 3: cache_destroy                                                   */
/* ======================================================================== */
#if STAGE >= 3

/* Double frees / invalid frees abort under ASan.
 * Leaks are detected by `make test-memcheck` (Valgrind, Linux container). */
static void test_destroy_repeated_cycles(void) {
  for (int i = 0; i < 1000; i++) {
    cache_t cache;
    cache_init(&cache, 4, 2, 4);
    cache_destroy(&cache);
  }
  CHECK(true);
}

static void test_destroy_single_set_single_line(void) {
  cache_t cache;
  cache_init(&cache, 0, 1, 0);
  cache_destroy(&cache);
  CHECK(true);
}

static void stage3(void) {
  puts("Stage 3: cache_destroy");
  RUN(test_destroy_repeated_cycles);
  RUN(test_destroy_single_set_single_line);
}
#endif

/* ======================================================================== */
/* Stage 4: cache_access                                                    */
/* ======================================================================== */
#if STAGE >= 4

/* (s=4, E=1, b=4): low hex digit = offset, next hex digit = set, rest = tag */
static void test_access_cold_miss_returns_miss_only(void) {
  cache_t c;
  cache_init(&c, 4, 1, 4);
  CHECK_EQ(cache_access(&c, 0x10, 0), CACHE_MISS);
  cache_destroy(&c);
}

static void test_access_same_block_different_offset_hits(void) {
  cache_t c;
  cache_init(&c, 4, 1, 4);
  cache_access(&c, 0x10, 0);
  CHECK_EQ(cache_access(&c, 0x10, 1), CACHE_HIT);
  CHECK_EQ(cache_access(&c, 0x1F, 2), CACHE_HIT); /* offset bits ignored */
  cache_destroy(&c);
}

static void test_access_decodes_set_index_and_tag(void) {
  cache_t c;
  cache_init(&c, 4, 1, 4);
  cache_access(&c, 0x3A5, 0); /* tag=0x3, set=0xA, offset=0x5 */
  CHECK(c.sets[0xA].lines[0].valid);
  CHECK_EQ(c.sets[0xA].lines[0].tag, 0x3);
  for (int i = 0; i < 16; i++) {
    if (i != 0xA) {
      CHECK(!c.sets[i].lines[0].valid); /* no other set touched */
    }
  }
  cache_destroy(&c);
}

static void test_access_tag_uses_full_64_bits(void) {
  cache_t c;
  cache_init(&c, 5, 1, 5);
  uint64_t addr = 0x7ff123456789ULL;
  uint64_t set = (addr >> 5) & 0x1F;
  cache_access(&c, addr, 0);
  CHECK(c.sets[set].lines[0].valid);
  CHECK(c.sets[set].lines[0].tag == (addr >> 10));
  cache_destroy(&c);
}

static void test_access_different_sets_do_not_conflict(void) {
  cache_t c;
  cache_init(&c, 4, 1, 4);
  CHECK_EQ(cache_access(&c, 0x10, 0), CACHE_MISS);
  CHECK_EQ(cache_access(&c, 0x20, 1), CACHE_MISS); /* set 2, no eviction */
  CHECK_EQ(cache_access(&c, 0x10, 2), CACHE_HIT);
  CHECK_EQ(cache_access(&c, 0x20, 3), CACHE_HIT);
  cache_destroy(&c);
}

static void test_access_direct_mapped_conflict_evicts(void) {
  cache_t c;
  cache_init(&c, 4, 1, 4);
  cache_access(&c, 0x010, 0);                                   /* set 1 */
  CHECK_EQ(cache_access(&c, 0x110, 1), CACHE_MISS | CACHE_EVICT); /* set 1 */
  CHECK_EQ(c.sets[1].lines[0].tag, 0x1);
  CHECK_EQ(cache_access(&c, 0x010, 2), CACHE_MISS | CACHE_EVICT); /* thrash */
  cache_destroy(&c);
}

static void test_access_fills_all_ways_before_evicting(void) {
  cache_t c;
  cache_init(&c, 0, 4, 0); /* 1 set, 4 ways, 1-byte blocks */
  for (int i = 0; i < 4; i++) {
    CHECK_EQ(cache_access(&c, (uint64_t)i, i), CACHE_MISS);
  }
  CHECK_EQ(cache_access(&c, 4, 4), CACHE_MISS | CACHE_EVICT);
  cache_destroy(&c);
}

static void test_access_evicts_least_recently_used(void) {
  cache_t c;
  cache_init(&c, 0, 2, 0);
  cache_access(&c, 0xA, 0);
  cache_access(&c, 0xB, 1);
  cache_access(&c, 0xC, 2);                    /* evicts A (oldest) */
  CHECK_EQ(cache_access(&c, 0xB, 3), CACHE_HIT); /* B survived */
  CHECK_EQ(cache_access(&c, 0xA, 4), CACHE_MISS | CACHE_EVICT);
  cache_destroy(&c);
}

static void test_access_hit_refreshes_recency(void) {
  cache_t c;
  cache_init(&c, 0, 2, 0);
  cache_access(&c, 0xA, 0);
  cache_access(&c, 0xB, 1);
  CHECK_EQ(cache_access(&c, 0xA, 2), CACHE_HIT); /* A now most recent */
  cache_access(&c, 0xC, 3);                      /* must evict B, not A */
  CHECK_EQ(cache_access(&c, 0xA, 4), CACHE_HIT);
  CHECK_EQ(cache_access(&c, 0xB, 5), CACHE_MISS | CACHE_EVICT);
  cache_destroy(&c);
}

static void test_access_yi_trace_sequence(void) {
  /* traces/yi.trace with (s=4, E=1, b=4); M = load then store. */
  cache_t c;
  cache_init(&c, 4, 1, 4);
  int clk = 0;
  CHECK_EQ(cache_access(&c, 0x10, clk++), CACHE_MISS);               /* L 10 */
  CHECK_EQ(cache_access(&c, 0x20, clk++), CACHE_MISS);               /* M 20 */
  CHECK_EQ(cache_access(&c, 0x20, clk++), CACHE_HIT);
  CHECK_EQ(cache_access(&c, 0x22, clk++), CACHE_HIT);                /* L 22 */
  CHECK_EQ(cache_access(&c, 0x18, clk++), CACHE_HIT);                /* S 18 */
  CHECK_EQ(cache_access(&c, 0x110, clk++), CACHE_MISS | CACHE_EVICT); /* L 110 */
  CHECK_EQ(cache_access(&c, 0x210, clk++), CACHE_MISS | CACHE_EVICT); /* L 210 */
  CHECK_EQ(cache_access(&c, 0x12, clk++), CACHE_MISS | CACHE_EVICT);  /* M 12 */
  CHECK_EQ(cache_access(&c, 0x12, clk++), CACHE_HIT);
  cache_destroy(&c);
}

static void stage4(void) {
  puts("Stage 4: cache_access");
  RUN(test_access_cold_miss_returns_miss_only);
  RUN(test_access_same_block_different_offset_hits);
  RUN(test_access_decodes_set_index_and_tag);
  RUN(test_access_tag_uses_full_64_bits);
  RUN(test_access_different_sets_do_not_conflict);
  RUN(test_access_direct_mapped_conflict_evicts);
  RUN(test_access_fills_all_ways_before_evicting);
  RUN(test_access_evicts_least_recently_used);
  RUN(test_access_hit_refreshes_recency);
  RUN(test_access_yi_trace_sequence);
}
#endif

int main(void) {
  printf("Running csim unit tests up to STAGE=%d\n", STAGE);
#if STAGE >= 1
  stage1();
#endif
#if STAGE >= 2
  stage2();
#endif
#if STAGE >= 3
  stage3();
#endif
#if STAGE >= 4
  stage4();
#endif
  printf("\n%d checks, %d failed\n", checks_run, checks_failed);
  return checks_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

