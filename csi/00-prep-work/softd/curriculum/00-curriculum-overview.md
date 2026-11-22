# Course Map and Philosophy

Advanced Software Design treats software engineering as applied logic, data modeling, and dependency geometry rather than memorization of frameworks or rigid object-oriented patterns. Code is the artifact; logic and invariants are the design.

# The 11 Aspects of Good Code

## External Properties

- Done: Meets functional requirements under real edge cases, not merely happy paths.
- Understandable: A reader can predict behavior without stepping through every execution trace.
- Evolvable: New requirements require localized edits instead of surgical rewrites across unrelated modules.

## Internal Properties

- Modularly understandable: A function or module can be verified from its specification and body alone, without reading callers or callees.
- Easy to recover programmer intent: Data types and names express why constraints exist, not just mechanical steps.
- Intent in a single place: Changing one policy decision touches one canonical representation rather than synchronized `if/else` ladders.
- Robust: Edge cases, partial failures, and unexpected orderings preserve invariants rather than leaving poisoned state.
- Hides secrets: Implementation choices (storage layout, encoding, indexing) stay behind narrow interfaces so callers cannot depend on them.
- Isolates assumptions: Contextual assumptions ("the World That Was") live at explicit boundaries rather than leaking across lines of logic.
- Open: New variants, backends, or rules plug in without editing existing dispatch code or breaking callers.
- Uses full wisdom: Uses the language's type system, algebraic structure, and formal reasoning tools rather than relying on discipline alone.

# Synthesis With Ousterhout and Jackson

## John Ousterhout — A Philosophy of Software Design

- Deep modules align directly with Parnas information hiding (Unit 4) and Hoare specifications (Unit 1): a narrow interface hides substantial internal complexity and invariants.
- Tactical vs. strategic programming explains **complexity ratchets** (Unit 6): every unsequestered assumption and defensive band-aid locks the codebase into a shallower, harder-to-refactor shape.
- Defining errors out of existence mirrors the **Representable/Valid Principle** (Unit 3): eliminating illegal states removes entire layers of error handling and defensive checks.

## Daniel Jackson — The Essence of Software

- Concept independence mirrors the **Embedded Design Principle** (Unit 2) and **Parnas Subsets** (Unit 6): independent concepts (e.g., transport routing vs. payload schema, read capability vs. cache eviction) must not pry into each other's internals.
- State modeling and purpose alignment map directly to **Algebraic Refactoring** (Unit 5) and **Data Over Code** (Unit 4): representing concept state explicitly as data lets synchronizations and policies stay inspectable and decoupled.

# Six-Unit Progression

- Unit 1 ([01-hidden-layer-of-logic.md](01-hidden-layer-of-logic.md)) — Expose hidden logic with Hoare triples and PIE failure tracing.
- Unit 2 ([02-embedded-design-principle.md](02-embedded-design-principle.md)) — Eliminate stringly coupling and prying couriers via parametric boundaries.
- Unit 3 ([03-representable-valid-principle.md](03-representable-valid-principle.md)) — Make illegal states unrepresentable so internal logic sheds defensive checks.
- Unit 4 ([04-data-over-code-principle.md](04-data-over-code-principle.md)) — Encapsulate design secrets and turn opaque closures into inspectable data.
- Unit 5 ([05-algebraic-refactoring.md](05-algebraic-refactoring.md)) — Factor data and control flow with sum/product cardinality laws.
- Unit 6 ([06-future-proofing-code.md](06-future-proofing-code.md)) — Apply RAD, behavioral subtyping (LSP), and Parnas subsets to resist complexity ratchets.
- Capstone ([07-capstone-guide.md](07-capstone-guide.md)) — Synthesize all six units in a mini container workload scheduler.

# Study Rhythm

- Spend 20 minutes reading a unit note and writing out the Active Recall answers before inspecting the code.
- Diagnose the starter lab strictly at the logic level first: identify every broken invariant, hidden assumption, or illegal state before writing Rust.
- Refactor incrementally, keeping `baseline_tests` green while flipping `lab_goals` tests from ignored to active one by one.
- Finish each unit by checking whether your final code satisfies the relevant subset of the 11 Aspects of Good Code.

# Active Recall

- What distinguishes an external property of good code from an internal property, and why can code that passes all current tests still fail modular understandability?
- How does Ousterhout's "define errors out of existence" map onto Koppel's Representable/Valid Principle?
- Why does Jackson's rule against concept entanglement reinforce Parnas information hiding?
