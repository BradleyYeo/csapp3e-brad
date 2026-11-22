# The Representable/Valid Principle

Make illegal states unrepresentable. Ideal data design maintains a one-to-one correspondence between states representable by your types and states valid in the domain.

# Counting the State Space

## Why Extra States Breed Bugs

- If your domain has 4 valid pod phases with phase-specific fields, packing them into one flat struct with `Option` fields and booleans creates thousands of representable combinations where almost all are nonsensical (e.g., `Pending` + `is_ready: true` + `exit_code: Some(0)`).
- Every illegal state forces every downstream function either to carry defensive `if/return Err` boilerplate or to suffer silent logic bugs when one check is forgotten.

## Eliminating Defensive Boilerplate

- Defensive checks inside core business logic are a symptom of an overly permissive data representation.
- Move checks once to the construction boundary; once inside valid types, core operations become total (infallible) functions requiring zero runtime defensive guards.

# Parse, Don't Validate

## Validation vs. Parsing

- Validation checks a permissive input (`&Pod`) and returns `Result<(), Error>`, throwing away proof of validity so the caller's types remain loose and the next function must validate again.
- Parsing checks input at the boundary and constructs a tighter, invariant-carrying type (`ReadyPod`, `NonZeroExitCode`), preserving proof of validity in the type system.

## Rust Boundary Tools

- Newtypes with private inner fields (`PodName(String)`, `NonZeroExitCode(NonZeroI32)`): impossible to forge invalid values outside the defining module.
- Enums per lifecycle phase (`PendingPod`, `RunningPod`, `SucceededPod`, `FailedPod`): each variant carries only the fields valid in that phase.
- Typestate or phase-specific references (`fn route_traffic(pod: &ReadyPod) -> [u8; 4]`): wrong-phase calls fail at compile time instead of returning runtime errors.

# Mental Model and Connections

- Ousterhout connection: Defining errors out of existence is sharpest at the type boundary—when `route_traffic` accepts `&ReadyPod`, the "pod not ready" and "missing IP" error cases disappear completely from its signature and body.
- Unit 3 lab preview: In [unit-03-representable-valid](../labs/unit-03-representable-valid/README.md), you replace a flat stringly-typed `Pod` struct full of defensive `Result` checks with boundary-parsed types and phase-exact variants verified by `compile_fail` doctests.

# Active Recall

- Explain the difference between "validating" an argument and "parsing" it into a stronger type.
- Why does tightening representable states to match valid states reduce defensive error-checking inside core functions?
- How do private fields inside a Rust module enable invariants that public struct fields cannot guarantee?
- When is a `compile_fail` doctest a stronger proof of design than a runtime unit test returning `Err`?
