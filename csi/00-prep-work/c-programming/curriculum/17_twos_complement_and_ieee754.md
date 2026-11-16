# Exercise 53: Two's Complement Arithmetic, Byte Order, and IEEE 754 Representation

Bridges C integer/bitwise drills to CS:APP Chapter 2 and `datalab`: endianness, signed-to-unsigned promotion bugs, undefined-behavior-free overflow detection, and IEEE 754 floating-point encoding.

---

# Endianness and Byte Representation

## Byte Ordering
- Multi-byte integers occupy contiguous bytes in virtual memory.
- Little-endian systems (x86-64, Apple Silicon ARM64) store the least significant byte at the lowest address.
- Network byte order (big-endian) stores the most significant byte first; `bswap_u32` converts between them using shifts and masks (`0xFFU`).

---

# Bit Manipulation Toolbox (the Drill 1 lessons)

## Hex Is Programmer's Binary
- One hex digit = one nibble = exactly 4 bits. There is no `%b` in C because there doesn't need to be one.
- Read `0x12345678` as four tiles — `12 | 34 | 56 | 78` — at positions 3, 2, 1, 0, where **position 0 = bits 7–0 (LSB end)**. Translate digit-by-digit, never through decimal.
- The digit↔nibble table (`5` = `0101`, `A` = `1010`, `F` = `1111`, …) must be automatic before Drills 5–6.

## Positions Are the Truth; Shifts Are Bookkeeping
- Every bitfield term answers one question: *which slot does this tile occupy in the final word?* Asserts check slots, not distances.
- With positions counted from the bottom: extract shift `s = 8 × source_position`, place shift `d = 8 × destination_position`.
- Two algebraically equivalent formulations — **pick one per expression, never mix**:
  - *extract-to-floor:* `((x >> s) & mask) << d` — the tile comes down to bits 7–0, gets masked, rises to its destination.
  - *travel-in-place:* `(x & (mask << s)) << (d − s)` — the mask sits at the source slot and the tile moves only by travel distance.
- The mask goes **between** the two shifts. Mask after placing ⇒ you delete exactly what you moved (the `0x00000034` bug). Mask before extracting ⇒ the mask can't see the target tile.

## Each Operator Has a Job
- `&` isolates; `|` assembles. `&` between two shifted copies of the same value intersects their bits — it returns a small number or zero, never a concatenation.
- `<<`/`>>` bind tighter than `&`/`|`, so `((x >> s) & mask) << d` needs its inner parens; write them anyway — verifying precedence costs more than typing it.
- One term per field, `|`-joined: each term is an independently printable probe, so a failed assert localizes to a tile, not to a soup of two variables named `back`/`front`.

## Why a Rotate Can't Be a Reverse (parity argument)
- `x << 16 | x >> 16` moves every byte **2 positions** — even. Byte reversal moves every byte **±1 or ±3** — odd. So halves swap but bytes never cross within a half: `0x12345678` → `0x5678 1234`, not `0x7856 3412`.
- The rotate is only rescuable by a second stage that swaps nibbles *within* every byte (`(r >> 4) & 0x0F0F0F0F | (r << 4) & 0xF0F0F0F0`) — a different two-stage algorithm (Route B), not an incrementally-fixed Route A.

## Debug-Printing Bits
- `printf("0x%08X", u)` — zero-pad to 8 digits; unpadded output hides *which byte slot* the wrong value sits in.
- Prints live in `main`/the test harness, never inside a pure helper: the function has a contract; the driver has curiosity.
- When one assert fails, print each term separately — the tile that lies names the bug.

---

# Two's Complement Hazards and Overflow Safety

## Unsigned Arithmetic Is Modular, Not "Non-Negative Numbers" (`CS:APP 2.25–2.26`, Drill 2)

### The C facts you need
- `strlen`'s contract (`<string.h>`): `size_t strlen(const char *s);` — `size_t` is an **unsigned** type, 64 bits on your machine.
- Unsigned arithmetic is **defined by ISO C to wrap modulo $2^{64}$** (C23 §6.2.5). There is no UB here and no warning — `2 - 3` on `size_t` is `SIZE_MAX` = `0xFFFFFFFFFFFFFFFF`. Verify by hand in binary: `0010 + ~0011 + 1 = 0010 + 1100 + 1 = 1111`, i.e. every bit set. That is the standard's design, not a bug in the hardware.
- For unsigned `u`, **`u > 0` is the same predicate as `u != 0`**. An unsigned value cannot be negative, so a subtraction's *sign* is no longer available as information — only its *zero-ness* survives.
- **Usual arithmetic conversions:** when a `size_t` and an `int` meet (in `-`, in `<`, anywhere), the `int` is converted to `size_t` first. So `int i = -1; i < 1;` is true, but `i < 1U;` is **false** — `-1` became $2^{64}-1$ before the comparison ever ran. This trap is silent even under `-Wsign-compare`-style flags when both sides look innocent, like `strlen(x) - n` where `n` is an `int`.

### Why the naive function is wrong
```c
return strlen(s) - strlen(t) > 0;   /* CS:APP 2.26 */
```
Compose the facts: the `-` lives in the modular ring, so it can't produce a negative; `> 0` on the result is really `!= 0`. The whole expression therefore means `strlen(s) != strlen(t)` — an equality question wearing a greater-than costume. Trace `("hi", "hello")`: `2 - 5 = SIZE_MAX`, `SIZE_MAX > 0` → `true`. The assert at your line 164 demands `false`. The bug is not "underflow sometimes happens" — the expression *always* computes the wrong predicate.

### The fix, and the principle behind it
- Direct fix: compare instead of subtract — `strlen(s) > strlen(t)`. No subtraction, no ring, no lost sign. A relational operator preserves the ordering information; a difference-then-test-sign destroys it. **Choose the operator that cannot lose the bit you're about to depend on.**
- Alternate fix (for later, when you genuinely need the difference — comparison callbacks, `qsort`): cast to the signed counterpart, `ptrdiff_t` (defined by `<stddef.h>` as the signed type wide enough for pointer differences). `(ptrdiff_t)strlen(s) - (ptrdiff_t)strlen(t)` restores modular $\equiv$ signed semantics for realistic string lengths.
- Generalized hazard to recognize on sight: **any API that returns "a magnitude" (`size_t`, `unsigned`, count-of-bytes-read) and any caller that asks "which direction" (`a - b > 0`, `a - b < 0`)**. You'll meet the same shape again in exercise 47 (`compare_files`) and in `qsort` comparator bugs everywhere.
- The null requirement is a *contract* item, not decoration: `strlen(NULL)` is UB, so the guard must run **before** any `strlen` call, and `||`'s left-to-right short-circuit order is what makes `if (s == nullptr || t == nullptr) return false;` legal to write on one line. The asserts at lines 166–167 pass `nullptr` precisely to punish an implementation that calls `strlen` first and checks after.

### Vocabulary: C23 booleans
- In C23 `bool`/`true`/`false` are keywords (`_Bool` before that; the `stdbool.h` shim at the top of this file covers pre-C23 compilers). Note the subtlety: C relational operators produce `int` 0/1, and converting to `bool` is well-defined — so `return strlen(s) > strlen(t);` in a `bool`-returning function is idiomatic, and comparing `== true` in the tests is belt-and-braces, not necessity.

## Overflow Without Undefined Behavior (`CS:APP 2.30`, Drill 3)

### The problem in one line
The "obvious" answer — `int32_t sum = x + y; return sum >= x ...;` or `return (x + y >= INT32_MIN) && ...` — computes `x + y` **signed**, and if it overflows, the program has *already* invoked UB before any check runs. You cannot inspect an overflowed signed sum; the standard promises nothing about what your code does next, so the check itself is meaningless.

### The C facts you need
- **Signed overflow is UB** (C23 §6.5.5p5: the result "is undefined" when it doesn't fit). **Unsigned overflow is defined** to reduce modulo $2^{32}$ (§6.2.5p9). The same `+` token has two different contracts depending on the operand type. This asymmetry is the whole exercise.
- **Why UB is worse than "crashes sometimes":** the compiler is entitled to *assume overflow never happens* and optimize on that premise. `x + 1 > x` can be rewritten to `true` for `int` — not because the optimizer is broken, but because the case where it's false requires UB, and UB is declared unreachable. (Real-world scar tissue: this exact reasoning removed bounds checks from production gzip.) The mental model (Ousterhout would call it a deep assumption hiding in plain sight): *UB is not "weird value"; UB is "your premise was false, and everything downstream, including dead-code elimination, follows from the compiler's world model."*
- **Two's complement makes the hardware identical for both types.** The ADD instruction produces the same 32 bits whether interpreted signed or unsigned; only the *reading* differs. So `(int32_t)((uint32_t)x + (uint32_t)y)` is the wrapped bit pattern, computed with defined behavior the entire way: two conversions into `uint32_t` (defined: add $2^{32}$ until in range), modular add (defined), conversion back to `int32_t` (defined in C23: the unique signed value congruent mod $2^{32}$ — C23 also *mandated* two's complement representations for all C compilers, settling a decades-long portability wart that older CS:APP editions treat as a footnote).
- `INT32_MAX` / `INT32_MIN` come from `<limits.h>`; the asserts use them, so know them by *bit pattern*, not just decimal: `0x7FFFFFFF` = `0 1111…1111`, `0x80000000` = `1 0000…0000`.

### The circle picture (why the sign test works)
Lay the 32-bit values on a circle of $2^{32}$ positions: top half (sign bit 0) = $[0, \text{INT32\_MAX}]$, bottom half (sign bit 1) = $[\text{INT32\_MIN}, -1]$, with `INT32_MAX` and `INT32_MIN` adjacent across the top boundary and across the bottom boundary. Addition walks clockwise; the wrapped sum is where you land *regardless*, but you have overflowed exactly when you crossed a half-boundary — and crossing is only possible if both starts are in the same half:
- Both ≥ 0 and the landing sum < 0 → crossed the top boundary → positive overflow.
- Both < 0 and the landing sum ≥ 0 → crossed the bottom boundary → negative overflow.
- Opposite signs: the walk goes counterclockwise from a positive or clockwise from a negative by an amount that can never leave the arc between them — $|x+y| \le \max(|x|, |y|)$ in the non-overflowing direction, so **mixed signs never overflow**. (This is why the predicate is *complete*: check the hint's two cases only. It's also why `INT32_MIN + INT32_MAX` in the assert list must be `true` — reason through where on the circle that walk goes.)

The detection predicate is therefore the two conjunctives from the stub hint, OR'd — computed on `x`, `y`, and the *wrapped* `s`, all of which exist with defined semantics.

### The elegant variant worth understanding (datalab flavor)
Branchless sign-flip detection: overflow happened iff `((x ^ s) & (y ^ s))` has its **sign bit set**. Read it as a sentence: "the sum disagrees in sign with *both* operands" — which by the circle picture can only happen by crossing a boundary. `a ^ b` is the *difference-of-bits* operator (1 where they disagree); you need only bit 31 of that, hence `< 0` instead of a mask. Derive on paper why `(x ^ s) < 0` alone is insufficient (`INT32_MIN + 1 + …`— find a case where one operand agrees in sign with the sum).

### Why not the alternative: `x > INT32_MAX - y`?
Pre-checking by rearranging the inequality (`(y > 0) ? (x > INT32_MAX - y) : (x < INT32_MIN - y)`) is a legitimate strategy — but audit it: `INT32_MIN - y` **itself overflows** when `y` is positive, so the "safe" formulation needs its own sign branch to be UB-free, and the cast-to-unsigned route is both shorter and mirrors what the hardware actually does. Lesson: a "no-UB" technique has to be *provably* no-UB, including its guard expressions.

### How your toolchain catches this
`-fsanitize=undefined` (in `make one` / `CFLAGS_SAN`) instruments signed overflow: a naive `x + y` implementation of Drill 3 will abort at the very first `INT32_MAX + 1`-style assert with a UBSan diagnostic pointing at the line — *even though the value might "look fine"*. When Drill 4's `saturating_add_i32` reuses your `tadd_ok_i32`, the sanitizer is what proves you kept the discipline of doing every arithmetic step in `uint32_t`.

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
- From memory, write the term that moves byte 1 of `x` into slot 3 of the result, and the one that moves byte 3 into slot 0. Which shift in each is extract and which is place?
- Where exactly does `& 0xFF` go in `((x >> 8) & 0xFF) << 16`, and why do both neighboring orders of operations lose information?
- Prove that no expression built from `x << 16` and `x >> 16` alone can produce a byte swap. (One sentence: name the invariant.)
- What predicate does `strlen(s) - strlen(t) > 0` *actually* compute, for all inputs? (Not "when does it break" — what is it always equal to, and why is that the more damning answer?)
- `int i = -1;` — why is `i < 1` true but `i < 1U` false? Name the rule and the moment the value changes.
- Why can `(uint32_t)x + (uint32_t)y` detect signed 32-bit overflow safely while `(x + y) < 0` cannot?
- Draw the $2^{32}$ circle. Mark `INT32_MAX`, `INT32_MIN`, and the walks for `INT32_MAX + 1`, `INT32_MIN + (-1)`, and `INT32_MIN + INT32_MAX`. Why does the third one terminate *between* the two halves and overflow-free?
- What exactly is the `(x ^ s) & (y ^ s) < 0` test reading off bit 31, and why is one XOR alone not enough?
- "Signed overflow is UB" — give the two-step argument for why that lets the compiler delete `if (sum < x)` rather than just making it unreliable.
- What is the bit pattern of `-0.0f` vs `+0.0f`, and why must `float_twice_bits` preserve the sign bit?
- What is the bit pattern of the largest denormalized `float`, and what happens when it is multiplied by `2.0f`?
