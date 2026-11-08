/*
 * Reference Solution: Exercise 01 (Cache Geometry & Bitwise Address Decoding)
 */
#include "../cache_sim.h"

void compute_cache_geometry(uint32_t s, uint32_t E, uint32_t b,
                            cache_geometry_t *out) {
  memset(out, 0, sizeof(*out));
  out->s = s;
  out->E = E;
  out->b = b;
  out->tag_bits = 64 - (s + b);
  out->num_sets = 1ULL << s;
  out->block_size = 1ULL << b;
  out->capacity_bytes = out->num_sets * E * out->block_size;
  out->critical_stride_bytes = out->num_sets * out->block_size;
}

decoded_addr_t decode_cache_address(uint64_t addr, uint32_t s, uint32_t b) {
  decoded_addr_t d;
  d.offset = addr & ((1ULL << b) - 1);
  d.set_index = (addr >> b) & ((1ULL << s) - 1);
  d.tag = addr >> (s + b);
  return d;
}

uint64_t decode_flawed_high_bit_index(uint64_t addr, uint32_t s) {
  return addr >> (64 - s);
}

int main(void) {
  printf("Running Reference Solution: Exercise 01...\n\n");

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

  cache_geometry_t geom_cl;
  compute_cache_geometry(5, 1, 5, &geom_cl);
  assert(geom_cl.num_sets == 32);
  assert(geom_cl.block_size == 32);
  assert(geom_cl.tag_bits == 54);
  assert(geom_cl.capacity_bytes == 1024);
  assert(geom_cl.critical_stride_bytes == 1024);

  decoded_addr_t dec = decode_cache_address(0x123456789ABCDEF5ULL, 4, 4);
  assert(dec.offset == 0x5ULL);
  assert(dec.set_index == 0xFULL);
  assert(dec.tag == 0x00123456789ABCDEULL);

  for (uint64_t block = 0; block < 16; ++block) {
    uint64_t addr = 0x400000ULL + block * 64ULL;
    decoded_addr_t mid = decode_cache_address(addr, 4, 6);
    uint64_t high = decode_flawed_high_bit_index(addr, 4);
    assert(mid.set_index == block);
    assert(high == 0ULL);
  }

  printf("Reference Solution 01 PASSED!\n");
  return EXIT_SUCCESS;
}

