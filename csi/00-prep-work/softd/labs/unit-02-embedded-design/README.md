# Unit 2 Lab — The Embedded Design Principle

Study [02-embedded-design-principle.md](../../curriculum/02-embedded-design-principle.md) before inspecting or refactoring [src/lib.rs](src/lib.rs).

# Scenario

A CI/CD release pipeline routes build artifacts through a chain of deployment stages and an `ArtifactRelay` courier. Currently every stage communicates via a flat `HashMap<String, String>` packed with `"env:prod|region:us-east"` formatted strings, stage flow is hardcoded by string dispatch, and `ArtifactRelay` silently inspects `"image"` names for `"hotfix-"` to override routing.

# Diagnosis Prompts

## Spotting Hidden Coupling

- Which types of coupling appear in `"env:prod|region:us-east"` and `"repo:tag"`?
- Why is `ArtifactRelay::dispatch` a prying courier, and what happens when a legitimate staged image happens to start with `"hotfix-"`?
- Why does adding a `Canary` stage require synchronized edits in multiple dispatch places?

# Refactoring Tasks

- Replace the flat `HashMap<String, String>` payload with explicit typed domain structs (`DeployTarget`, `Environment`, `ReleaseArtifact`).
- Introduce `RoutingEnvelope<T>` and make `ArtifactRelay` deliver generic `RoutingEnvelope<T>` payloads so Rust's parametricity makes prying into `T` impossible at compile time.
- Replace hardcoded stage string dispatch with a composable `PipelineStage` trait and `DeployPipeline` runner so a `Canary` stage plugs in without touching existing stage code or the courier.
- Update and un-ignore `lab_goals` tests (check [solutions/unit-02-embedded-design/src/lib.rs](../../solutions/unit-02-embedded-design/src/lib.rs) after completing your attempt).

# Commands

```sh
cargo test -p unit-02-embedded-design
cargo test -p unit-02-embedded-design -- --ignored
```
