/*
 * Exercise 01: Cache Geometry & Bitwise Address Decoding
 *
 * Guide: 01_address_decoding.md
 * Shared definitions: cache_sim.h
 *
 * Build and run:
 *   make 01
 */
#include "cache_sim.h"

/*
 * Drill 1: Compute Cache Geometry Parameters
 *
 * Precondition:
 *   s + b < 64, E >= 1, out != nullptr.
 * Postcondition:
 *   Populates all fields of *out:
 *   - s, E, b stored directly.
 *   - num_sets = 2^s
 *   - block_size = 2^b
 *   - tag_bits = 64 - (s + b)
 *   - capacity_bytes = S * E * B
 *   - critical_stride_bytes = S * B
 */
void compute_cache_geometry(uint32_t s, uint32_t E, uint32_t b,
                            cache_geometry_t *out) {
  // TODO: Implement cache geometry calculation using 1ULL
  (void)s;
  (void)E;
  (void)b;
  memset(out, 0, sizeof(*out));
}

/*
 * Drill 2: Decode 64-Bit Address Components
 *
 * Precondition:
 *   s + b < 64.
 * Postcondition:
 *   Returns decoded_addr_t with:
 *   - offset: bits [0 .. b-1]
 *   - set_index: bits [b .. b+s-1]
 *   - tag: bits [b+s .. 63]
 */
decoded_addr_t decode_cache_address(uint64_t addr, uint32_t s, uint32_t b) {
  // TODO: Extract offset, set_index, and tag using bitwise shift and mask
  (void)addr;
  (void)s;
  (void)b;
  return (decoded_addr_t){0, 0, 0};
}

/*
 * Drill 3: Extract Flawed High-Bit Set Index
 *
 * Precondition:
 *   s > 0 && s < 64.
 * Postcondition:
 *   Returns bits [64-s .. 63] of addr.
 */
uint64_t decode_flawed_high_bit_index(uint64_t addr, uint32_t s) {
  // TODO: Shift top s bits down to position 0
  (void)addr;
  (void)s;
  return 0;
}

int main(void) {
  printf("Running Exercise 01: Cache Geometry & Address Decoding...\n\n");

  /* Test 1: Typical L1d cache geometry (s=6, E=8, b=6) -> 32 KiB */
  cache_geometry_t geom_l1;
  compute_cache_geometry(6, 8, 6, &geom_l1);
  assert(geom_l1.s == 6);
  assert(geom_l1.E == 8);
  assert(geom_l1.b == 6);
  assert(geom_l1.num_sets == 64);
  assert(geom_l1.block_size == 64);
  assert(geom_l1.tag_bits == 52);
  assert(geom_l1.capacity_bytes == 32768);
  assert(geom_l1.critical_stride_bytes == 4096);

  /* Test 2: Cache Lab Part B geometry (s=5, E=1, b=5) -> 1 KiB */
  cache_geometry_t geom_cl;
  compute_cache_geometry(5, 1, 5, &geom_cl);
  assert(geom_cl.num_sets == 32);
  assert(geom_cl.block_size == 32);
  assert(geom_cl.tag_bits == 54);
  assert(geom_cl.capacity_bytes == 1024);
  assert(geom_cl.critical_stride_bytes == 1024);

  /* Test 3: Address decoding on (s=4, b=4) */
  decoded_addr_t dec = decode_cache_address(0x123456789ABCDEF5ULL, 4, 4);
  assert(dec.offset == 0x5ULL);
  assert(dec.set_index == 0xFULL);
  assert(dec.tag == 0x00123456789ABCDEULL);

  /* Test 4: Middle-Bit vs High-Bit Indexing across contiguous 64-byte blocks */
  for (uint64_t block = 0; block < 16; ++block) {
    uint64_t addr = 0x400000ULL + block * 64ULL;
    decoded_addr_t mid = decode_cache_address(addr, 4, 6);
    uint64_t high = decode_flawed_high_bit_index(addr, 4);
    assert(mid.set_index == block); /* Middle bits cycle cleanly across sets 0..15 */
    assert(high == 0ULL);           /* High bits all collide in Set 0! */
  }

  printf("Exercise 01 PASSED!\n");
  return EXIT_SUCCESS;
}

