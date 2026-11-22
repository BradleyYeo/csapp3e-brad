# Advanced Software Design Curriculum and Labs

A self-contained Rust workspace modeling the six-unit Mirdin *Advanced Software Design* curriculum by Jimmy Koppel, integrated with John Ousterhout (*A Philosophy of Software Design*) and Daniel Jackson (*The Essence of Software*). Every crate uses Rust `std` only so all compilation, refactoring checks, and `compile_fail` doctests run offline.

# Workspace Structure

- `curriculum/`: Obsidian-compatible study notes covering mental models, formal tools, and active recall prompts.
- `labs/`: Flawed starter crates with passing `baseline_tests` and ignored `lab_goals` tests that fail until you complete the refactoring.
- `solutions/`: Independent Cargo workspace containing reference refactored implementations and `compile_fail` doctests.

# Curriculum Roadmap

## Core Units

- [00-curriculum-overview.md](curriculum/00-curriculum-overview.md) — Roadmap, 11 Aspects of Good Code, and synthesis with Ousterhout and Jackson
- [01-hidden-layer-of-logic.md](curriculum/01-hidden-layer-of-logic.md) — Three Levels of Software, Hoare triples, invariants, and PIE debugging
- [02-embedded-design-principle.md](curriculum/02-embedded-design-principle.md) — Mechanical derivation, coupling taxonomy, and opaque couriers
- [03-representable-valid-principle.md](curriculum/03-representable-valid-principle.md) — State-space bijection, Parse Don't Validate, and typestate
- [04-data-over-code-principle.md](curriculum/04-data-over-code-principle.md) — Parnas information hiding and defunctionalization
- [05-algebraic-refactoring.md](curriculum/05-algebraic-refactoring.md) — Sum/product cardinality, polynomial factoring, and unified patterns
- [06-future-proofing-code.md](curriculum/06-future-proofing-code.md) — RAD process, behavioral subtyping (LSP), and Parnas subsets
- [07-capstone-guide.md](curriculum/07-capstone-guide.md) — Capstone integration brief and 11-Aspect review rubric

## Hands-On Labs

- Unit 1 ([unit-01-hidden-logic](labs/unit-01-hidden-logic/README.md)) — Token-bucket rate limiter and quota tracker (`solutions/unit-01-hidden-logic`)
- Unit 2 ([unit-02-embedded-design](labs/unit-02-embedded-design/README.md)) — CI/CD release pipeline and artifact courier (`solutions/unit-02-embedded-design`)
- Unit 3 ([unit-03-representable-valid](labs/unit-03-representable-valid/README.md)) — Container pod lifecycle and network lease (`solutions/unit-03-representable-valid`)
- Unit 4 ([unit-04-data-over-code](labs/unit-04-data-over-code/README.md)) — Observability alert rules and Parnas event index (`solutions/unit-04-data-over-code`)
- Unit 5 ([unit-05-algebraic-refactoring](labs/unit-05-algebraic-refactoring/README.md)) — Cloud access-control and permission algebra (`solutions/unit-05-algebraic-refactoring`)
- Unit 6 ([unit-06-future-proofing](labs/unit-06-future-proofing/README.md)) — Pluggable checkpoint store (`solutions/unit-06-future-proofing`)
- Capstone ([capstone-scheduler](labs/capstone-scheduler/README.md)) — Mini container workload scheduler and policy engine (`solutions/capstone-scheduler`)

# Deliberate Practice Workflow

## Per-Unit Loop

- Read the unit note in `curriculum/` and answer the Active Recall section without looking back.
- Inspect the starter crate in `labs/<unit>/README.md` and `labs/<unit>/src/lib.rs`.
- Run `cargo test -p <unit>` to verify the baseline tests pass.
- Run `cargo test -p <unit> -- --ignored` to watch the `lab_goals` tests fail on the planted flaws.
- Refactor `labs/<unit>/src/lib.rs`, remove the `#[ignore]` attributes from `lab_goals`, and run `cargo test -p <unit>` until all tests and doctests pass with zero warnings.
- Compare your design against `solutions/<unit>/src/lib.rs` and critique tradeoffs against the 11 Aspects of Good Code.

## Workspace Commands

```sh
# Run baseline starter tests across all labs
cargo test

# Reveal planted starter failures across all labs
cargo test -- --ignored

# Run single lab (including ignored lab_goals)
cargo test -p unit-01-hidden-logic -- --include-ignored

# Verify all reference solutions and compile_fail doctests
cargo test --manifest-path solutions/Cargo.toml
```
