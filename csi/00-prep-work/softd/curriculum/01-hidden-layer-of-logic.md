# The Hidden Layer of Logic

Code that never crashes on existing tests can still be broken. Design lives in the logical relationships between states and operations, not in syntax formatting or class hierarchies.

# The Three Levels of Software

## Level Breakdown

- Runtime level: Concrete execution traces, memory contents, latencies, and crashes observed during a specific run.
- Code level: The concrete syntax, control flow, structs, and functions written in source files.
- Logic (Design) level: Specifications, invariants, preconditions, postconditions, and modular guarantees that hold across all valid inputs.

## Errors of Modular Reasoning

- A function has an error of modular reasoning when it relies on an undocumented runtime condition from callers or breaks an expected guarantee for future callers, even if current integration tests pass.
- Testing only samples the Runtime level; Hoare logic reasons over the entire Logic level at once.

# Hoare Triples and Invariants

## Anatomy of a Hoare Triple

- Written `{P} C {Q}`: if precondition `P` holds before executing command `C`, and `C` terminates, then postcondition `Q` holds afterward.
- Weakening a precondition (`P` requires less) makes a function easier and safer to call.
- Strengthening a postcondition (`Q` guarantees more) gives callers stronger building blocks.

## Struct and Module Invariants

- An invariant `I` is a predicate that must hold in both `P` and `Q` for every public operation on a state-bearing type (`{P && I} op {Q && I}`).
- Partial state mutation on error breaks `I`: if an operation returns `Err`, callers expect either full state transition or unchanged valid state (strong exception/rollback guarantee), never half-deducted balances.

## Expressing Hoare Logic in Rust

- Document `Precondition`, `Postcondition`, and `Invariant` directly on methods.
- Guard internal state boundaries with `debug_assert!` invariant checks during tests.
- Enforce preconditions through types where possible (Unit 3) so invalid `{P}` cannot even compile.

# The PIE Model and Hoare-Guided Debugging

## Defect, Infection, Failure

- Defect: The flawed line or missing guard in source code (e.g., updating a watermark unconditionally).
- Infection: A corrupted internal state (`tokens`, `watermark`, `last_refill_ms`) produced when execution hits the defect, before any external symptom appears.
- Failure: An observable wrong return value, panic, or broken postcondition noticed much later.

## Using Hoare Triples With Printouts

- Random printouts show runtime noise; Hoare-guided printouts test the exact boundary where `{P}` holds entering a block but `{Q}` or invariant `I` breaks leaving it.
- Binary-searching the execution path for the first point where `I` becomes false pinpoints the exact transition from Defect to Infection.

# Mental Model and Connections

- Ousterhout connection: Deep modules expose simple `{P}` and `{Q}` contracts while preserving complex internal invariants without forcing callers to track execution history.
- Unit 1 lab preview: In [unit-01-hidden-logic](../labs/unit-01-hidden-logic/README.md), a rate limiter passes happy-path tests while hiding timestamp rollback bugs, partial batch mutation, truncating wait-time arithmetic, and delayed watermark infection.

# Active Recall

- Why can code that passes 100 percent of current runtime tests still violate modular reasoning?
- In `{P} C {Q}`, what happens to caller coupling when you weaken `P` versus when you weaken `Q`?
- Distinguish a Defect, an Infection, and a Failure in a multi-call stateful API such as a rate limiter or ledger.
- When an operation returns `Err`, what postcondition on the receiver's invariant must still hold?
