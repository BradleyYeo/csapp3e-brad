# Unit 1 Lab — Hidden Layer of Logic

Study [01-hidden-layer-of-logic.md](../../curriculum/01-hidden-layer-of-logic.md) before inspecting or refactoring [src/lib.rs](src/lib.rs).

# Scenario

`RateLimiter` manages a token bucket with a configurable capacity, refill rate, timestamp watermark, and cumulative usage reconciler. Existing `baseline_tests` pass completely, yet four logic-level defects lurk beneath the surface.

# Diagnosis Prompts

## Inspecting at the Logic Level

- State the struct invariant `I` linking `tokens` and `capacity`. Which method lets callers violate `I`?
- What precondition on `now_ms` does `refill_and_take` silently assume, and what happens when clock skew passes `now_ms < last_refill_ms`?
- Why does `reserve_batch` break the strong rollback postcondition when the third cost exceeds remaining tokens?
- Write the Hoare postcondition relating `effective_wait_ms(cost)` and `refill_and_take(now + wait, cost)`. Why does integer truncation break it?
- Trace `reconcile_usage` across three calls `(10 -> 4 -> 9)` using the PIE model (Defect, Infection, Failure).

# Refactoring Tasks

- Document explicit `Precondition`, `Postcondition`, and `Invariant` Hoare contracts on every public method of `RateLimiter`.
- Add a private `assert_invariants(&self)` helper using `debug_assert!` at entry/exit boundaries.
- Clamp `credit_tokens` so `tokens <= capacity` always holds.
- Handle non-monotonic timestamps (`now_ms < last_refill_ms`) without corrupting `last_refill_ms` or losing tokens.
- Make `reserve_batch` atomic so a failed reservation leaves `tokens` unchanged.
- Fix `effective_wait_ms` with ceiling arithmetic so waiting `w` ms guarantees `refill_and_take(now + w, cost)` succeeds when `cost <= capacity`.
- Fix `reconcile_usage` so an out-of-order lower total does not infect `usage_watermark`.
- Remove `#[ignore]` from every test in `lab_goals` and verify all tests pass.

# Commands

```sh
# Confirm baseline passes
cargo test -p unit-01-hidden-logic

# Reveal the planted logic failures
cargo test -p unit-01-hidden-logic -- --ignored

# Verify everything after removing #[ignore]
cargo test -p unit-01-hidden-logic
```
