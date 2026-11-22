# Unit 5 Lab — Algebraic Refactoring

Study [05-algebraic-refactoring.md](../../curriculum/05-algebraic-refactoring.md) before inspecting or refactoring [src/lib.rs](src/lib.rs).

# Scenario

An IAM authorization evaluator models grants as a bloated product struct (`AccessGrant`) combining `PrincipalKind`, four `Option` credential fields, and two permission booleans (`is_readonly`, `allow_exec`). Because common envelope logic (`expires_at`, `scope`) is duplicated across nested `if/else` arms inside `evaluate_access`, the `BreakGlass` arm forgets to check expiration while contradictory permission pairs slip through `try_new`.

# Diagnosis Prompts

## Counting Inhabitants and Factoring Polynomials

- How many boolean permission states can `(is_readonly: bool, allow_exec: bool)` represent vs. how many are valid (`ReadOnly`, `ReadWrite`, `AdminExec`)?
- How many combinations of `(PrincipalKind, Option<bool>, Option<String>, Option<String>, Option<String>)` exist if `Option<T>` multiplies out independent fields instead of forming a sum type?
- How does factoring `GrantEnvelope { scope, expires_at, permission, principal: PrincipalCredential }` (`A * (B + C + D)`) eliminate duplicated expiration and scope checks?

# Refactoring Tasks

- Collapse `(is_readonly, allow_exec)` into a three-variant `PermissionLevel` enum (`ReadOnly`, `ReadWrite`, `AdminExec`).
- Refactor `PrincipalCredential` into an exact sum type (`Human`, `ServiceAccount`, `BreakGlass`) carrying only the fields valid for each principal.
- Factor common metadata (`grant_id`, `scope`, `expires_at`, `permission`) into an outer `GrantEnvelope` struct wrapping `PrincipalCredential`.
- Factor `evaluate_access` so expiration and scope checks happen once above the `match`.
- Update `lab_goals` (and verify `compile_fail` doctests in [solutions/unit-05-algebraic-refactoring/src/lib.rs](../../solutions/unit-05-algebraic-refactoring/src/lib.rs) for comparison).

# Commands

```sh
cargo test -p unit-05-algebraic-refactoring
cargo test -p unit-05-algebraic-refactoring -- --ignored
```
