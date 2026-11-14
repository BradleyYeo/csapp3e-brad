# Exercise 53: Two's Complement Arithmetic, Byte Order, and IEEE 754 Representation

Bridges C integer/bitwise drills to CS:APP Chapter 2 and `datalab`: endianness, signed-to-unsigned promotion bugs, undefined-behavior-free overflow detection, and IEEE 754 floating-point encoding.

---

# Endianness and Byte Representation

## Byte Ordering
- Multi-byte integers occupy contiguous bytes in virtual memory.
- Little-endian systems (x86-64, Apple Silicon ARM64) store the least significant byte at the lowest address.
- Network byte order (big-endian) stores the most significant byte first; `bswap_u32` converts between them using shifts and masks (`0xFFU`).

---

# Two's Complement Hazards and Overflow Safety

## Implicit Unsigned Promotion (`CS:APP 2.25–2.26`)
- Mixing `int` and `size_t` promotes the signed operand to unsigned.
- `strlen(s) - strlen(t) > 0` underflows to a huge `size_t` value whenever `strlen(s) < strlen(t)`, returning `true` instead of `false`.
- Fix by comparing directly (`strlen(s) > strlen(t)`).

## Overflow Without Undefined Behavior
- Signed integer overflow (`INT32_MAX + 1`) is Undefined Behavior in C; compilers may optimize away post-overflow checks like `x + y < x`.
- Unsigned addition modulo $2^{32}$ is fully defined by ISO C.
- Cast operands to `uint32_t`, add, cast back to `int32_t`, and check whether two same-signed inputs produced a sum of the opposite sign.

---

# IEEE 754 Single-Precision Layout (`float_twice`)

```
31      30           23 22                                0
+------+---------------+----------------------------------+
| sign |   exp (8-bit) |          frac (23-bit)           |
+------+---------------+----------------------------------+
```

## Five Value Classes
- Zero (`exp == 0`, `frac == 0`): includes both `+0.0` (`0x00000000`) and `-0.0` (`0x80000000`).
- Denormalized (`exp == 0`, `frac != 0`): implicit leading bit is `0`; value is $(-1)^s \times 0.\text{frac} \times 2^{-126}$.
- Normalized (`0 < exp < 255`): implicit leading bit is `1`; value is $(-1)^s \times 1.\text{frac} \times 2^{\text{exp}-127}$.
- Infinity (`exp == 255`, `frac == 0`): signed overflow saturation value.
- NaN (`exp == 255`, `frac != 0`): propagates unchanged through arithmetic.

## Why Doubling Denormals Works by Shifting `frac`
- Both denormals (`exp == 0`) and the smallest normal (`exp == 1`) share the exact same exponent weight $2^{-126}$.
- Shifting `frac` left by 1 bit when `exp == 0` moves a carry from bit 22 directly into bit 23 (`exp` becomes `1`), smoothly crossing the denormal-to-normal boundary without discontinuity.

---

# Active Recall Questions
- Why can `(uint32_t)x + (uint32_t)y` detect signed 32-bit overflow safely while `(x + y) < 0` cannot?
- What is the bit pattern of `-0.0f` vs `+0.0f`, and why must `float_twice_bits` preserve the sign bit?
- What is the bit pattern of the largest denormalized `float`, and what happens when it is multiplied by `2.0f`?
