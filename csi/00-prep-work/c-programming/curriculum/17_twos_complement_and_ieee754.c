/*
 * Two's Complement Arithmetic, Byte Order & IEEE 754 Bits (CS:APP Ch. 2 /
 * datalab)
 *
 * Compile and run:
 *   make -C curriculum 53_twos_complement_and_ieee754 &&
 * ./curriculum/53_twos_complement_and_ieee754
 */
#define _POSIX_C_SOURCE 200809L
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
#include <stdbool.h>
#ifndef nullptr
#define nullptr NULL
#endif
#endif

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  uint8_t sign;
  uint8_t exp;
  uint32_t frac;
} FloatParts;

typedef enum { FP_ZERO = 0, FP_DENORM, FP_NORM, FP_INF, FP_NAN } FloatClass;

/*
 * Drill 1: Byte Swapping (Endianness Conversion)
 *
 * Requirements:
 * - Reverse the 4 bytes of x using shifts and bitmasks (no compiler
 * intrinsics).
 * - Example: 0x12345678U -> 0x78563412U.
 */
static uint32_t bswap_u32(uint32_t x) {
  uint32_t mask = 0xFFU;

  return ((x >> 0) & mask) << 24 | ((x >> 8) & mask) << 16 |
         ((x >> 16) & mask) << 8 | ((x >> 24) & mask) << 0;
}

/*
 * Drill 2: Unsigned Promotion Bug (CS:APP Practice Problem 2.26)
 *
 * Naive broken version:
 *   return strlen(s) - strlen(t) > 0;
 * Since strlen returns unsigned size_t, if strlen(s) < strlen(t) the
 * subtraction wraps around to a huge positive value and returns true!
 *
 * Requirements:
 * - Return false if s == nullptr || t == nullptr.
 * - Return true iff strlen(s) > strlen(t) without unsigned underflow.
 */
static bool safe_str_longer(const char *s, const char *t) {
  if (s == nullptr || t == nullptr) {
    return false; /* TODO: implement */
  }
  return strlen(s) > strlen(t);
}

/*
 * Drill 3: UB-Free Two's Complement Overflow Detection (CS:APP 2.30)
 *
 * Requirements:
 * - Return true iff x + y can be represented in int32_t without overflow.
 * - MUST NOT perform signed `x + y` before proving no overflow (signed overflow
 * is UB!).
 * - Hint: compute `uint32_t sum = (uint32_t)x + (uint32_t)y; int32_t s =
 * (int32_t)sum;` Positive overflow occurs iff x >= 0 && y >= 0 && s < 0.
 *   Negative overflow occurs iff x < 0 && y < 0 && s >= 0.
 */
static bool tadd_ok_i32(int32_t x, int32_t y) {
  uint32_t sum = (uint32_t)x + (uint32_t)y;
  int32_t s = (int32_t)sum;
  bool overflowed =
      !((x >= 0 && y >= 0 && s < 0) || (x < 0 && y < 0 && s >= 0));

  return overflowed;
}

/*
 * Drill 4: Two's Complement Saturating Addition (datalab-style)
 *
 * Requirements:
 * - Return INT32_MAX on positive overflow.
 * - Return INT32_MIN on negative overflow.
 * - Otherwise return x + y (computed via unsigned arithmetic so UBSan never
 * fires).
 */
static int32_t saturating_add_i32(int32_t x, int32_t y) {
  uint32_t sum = (uint32_t)x + (uint32_t)y;
  int32_t s = (int32_t)sum;
  if (x >= 0 && y >= 0 && s < 0) {
    return INT32_MAX;
  }
  if (x < 0 && y < 0 && s >= 0) {
    return INT_MIN;
  }
  return s;
}

/*
 * Drill 5: IEEE 754 Single-Precision Decomposition & Classification
 *
 * Layout (32 bits): [ sign (1 bit: 31) | exp (8 bits: 30..23) | frac (23
 * bits: 22..0) ]
 *
 * Requirements for float_decompose:
 * - Extract sign (0 or 1), exp (0..255), and frac (0..0x7FFFFF).
 *
 * Requirements for float_classify:
 * - exp == 0 && frac == 0   -> FP_ZERO
 * - exp == 0 && frac != 0   -> FP_DENORM
 * - exp == 0xFF && frac == 0 -> FP_INF
 * - exp == 0xFF && frac != 0 -> FP_NAN
 * - otherwise               -> FP_NORM
 */
static FloatParts float_decompose(uint32_t bits) {
  (void)bits;
  uint32_t sign = bits >> 31 & 0x1U;
  uint32_t exp = bits >> 23 & 0xFFU;
  uint32_t frac = bits & 0x7FFFFFU;
  return (FloatParts){sign = sign, exp = exp,
                      frac = frac};
}

static FloatClass float_classify(uint32_t bits) {
  (void)bits;
  return FP_ZERO; /* TODO: implement */
}

/*
 * Drill 6: Bit-Level IEEE 754 Doubling (`float_twice` from CS:APP datalab)
 *
 * Compute bit-level equivalent of expression `2.0f * f` for floating-point
 * argument `f` represented as raw uint32_t bits `uf`.
 *
 * Requirements:
 * - If NaN or Inf (exp == 0xFF), return `uf` unchanged.
 * - If Zero or Denormalized (exp == 0):
 *     Preserve sign bit, shift frac left by 1 (`frac <<= 1`).
 *     Notice: if bit 22 was 1, shifting left puts 1 into bit 23 (exp becomes
 * 1), seamlessly transitioning from denormalized to normalized!
 * - If Normalized (0 < exp < 0xFF):
 *     Increment exp by 1.
 *     If exp reaches 0xFF (overflow to infinity), set frac = 0.
 * - Reassemble `(sign << 31) | (exp << 23) | frac` and return.
 */
static uint32_t float_twice_bits(uint32_t uf) {
  (void)uf;
  return 0U; /* TODO: implement */
}

static uint32_t f32_to_bits(float f) {
  uint32_t bits = 0;
  memcpy(&bits, &f, sizeof(bits));
  return bits;
}

int main(void) {
  /* Drill 1: Byte swap */
  printf("bswap(0x%08X) = 0x%08X, want 0x%08X\n", 0x12345678U,
         bswap_u32(0x12345678U), 0x78563412U);
  assert(bswap_u32(0x12345678U) == 0x78563412U);
  assert(bswap_u32(0x00FF0001U) == 0x0100FF00U);
  assert(bswap_u32(0xAABBCCDDU) == 0xDDCCBBAAU);
  puts("[PASS] Drill 1: byte swap");

  /* Drill 2: Unsigned promotion underflow */
  assert(safe_str_longer("hello", "hi") == true);
  assert(safe_str_longer("hi", "hello") == false);
  assert(safe_str_longer("abc", "def") == false);
  assert(safe_str_longer(nullptr, "abc") == false);
  assert(safe_str_longer("abc", nullptr) == false);
  puts("[PASS] Drill 2: unsigned promotion");

  /* Drill 3: Two's complement addition overflow detection */
  assert(tadd_ok_i32(100, 200) == true);
  assert(tadd_ok_i32(INT32_MAX, 0) == true);
  assert(tadd_ok_i32(INT32_MAX, 1) == false);
  assert(tadd_ok_i32(INT32_MAX, INT32_MAX) == false);
  assert(tadd_ok_i32(INT32_MIN, 0) == true);
  assert(tadd_ok_i32(INT32_MIN, -1) == false);
  assert(tadd_ok_i32(INT32_MIN, INT32_MAX) == true);
  puts("[PASS] Drill 3: overflow detection");

  /* Drill 4: Saturating addition */
  assert(saturating_add_i32(10, -3) == 7);
  assert(saturating_add_i32(INT32_MAX, 1) == INT32_MAX);
  assert(saturating_add_i32(INT32_MAX - 5, 10) == INT32_MAX);
  assert(saturating_add_i32(INT32_MIN, -1) == INT32_MIN);
  assert(saturating_add_i32(INT32_MIN + 5, -10) == INT32_MIN);
  puts("[PASS] Drill 4: saturating add");

  /* Drill 5: IEEE 754 decomposition & classification */
  uint32_t neg_one =
      f32_to_bits(-1.5f); /* 0xBFC00000: sign=1, exp=127, frac=0x400000 */
  FloatParts p = float_decompose(neg_one);
  assert(p.sign == 1 && p.exp == 127 && p.frac == 0x400000U);
  assert(float_classify(0x00000000U) == FP_ZERO);
  assert(float_classify(0x80000000U) == FP_ZERO); /* -0.0f */
  assert(float_classify(0x00000001U) == FP_DENORM);
  assert(float_classify(neg_one) == FP_NORM);
  assert(float_classify(0x7F800000U) == FP_INF);
  assert(float_classify(0xFF800000U) == FP_INF);
  assert(float_classify(0x7FC00001U) == FP_NAN);
  puts("[PASS] Drill 5: decompose & classify");

  /* Drill 6: float_twice_bits across all IEEE 754 categories */
  assert(float_twice_bits(0x00000000U) == 0x00000000U); /* +0.0 -> +0.0 */
  assert(float_twice_bits(0x80000000U) == 0x80000000U); /* -0.0 -> -0.0 */
  assert(float_twice_bits(0x00000001U) == 0x00000002U); /* small denorm */
  assert(float_twice_bits(0x007FFFFFU) ==
         0x00FFFFFEU); /* max denorm -> norm! */
  assert(float_twice_bits(0x807FFFFFU) ==
         0x80FFFFFEU); /* negative denorm -> norm! */
  assert(float_twice_bits(f32_to_bits(1.5f)) == f32_to_bits(3.0f));
  assert(float_twice_bits(f32_to_bits(-3.25f)) == f32_to_bits(-6.5f));
  assert(float_twice_bits(0x7F7FFFFFU) == 0x7F800000U); /* max norm -> +Inf */
  assert(float_twice_bits(0xFF7FFFFFU) == 0xFF800000U); /* -max norm -> -Inf */
  assert(float_twice_bits(0x7F800000U) == 0x7F800000U); /* +Inf -> +Inf */
  assert(float_twice_bits(0x7FC01234U) == 0x7FC01234U); /* NaN -> NaN */
  puts("[PASS] Drill 6: float_twice_bits");

  printf("[PASS] Exercise 53: Two's Complement & IEEE 754 Bits verified.\n");
  return EXIT_SUCCESS;
}
