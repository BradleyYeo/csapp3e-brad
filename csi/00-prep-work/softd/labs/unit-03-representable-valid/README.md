# Unit 3 Lab — The Representable/Valid Principle

Study [03-representable-valid-principle.md](../../curriculum/03-representable-valid-principle.md) before inspecting or refactoring [src/lib.rs](src/lib.rs).

# Scenario

A container pod lifecycle module represents every pod as one flat `Pod` struct with `phase: String`, multiple `Option` fields, and `is_ready: bool`. Because thousands of illegal combinations are representable, functions like `route_traffic` and `failure_summary` are stuffed with defensive `Result` checks, while `Pod::from_spec` accepts contradictory pods silently.

# Diagnosis Prompts

## Counting Representable vs. Valid States

- Which combinations of `phase`, `container_id`, `ip_addr`, `exit_code`, `failure_reason`, and `is_ready` are invalid in a container lifecycle?
- Why does `route_traffic(&Pod) -> Result<[u8; 4], String>` keep needing defensive checks even if every caller validates beforehand?
- What boundary types eliminate empty pod names, zero exit codes on failed pods, and `restart_count > max_restarts`?

# Refactoring Tasks

- Introduce private-field boundary types (`PodName`, `ContainerId`, `NonZeroExitCode`, `RestartBudget`).
- Partition the lifecycle into exact per-phase structs (`PendingPod`, `RunningPod`, `ReadyPod`, `SucceededPod`, `FailedPod`) unified under `PodPhase`.
- Refactor `route_traffic` and `failure_summary` to accept `&ReadyPod` and `&FailedPod` so they become total, infallible functions with zero defensive checks.
- Add `compile_fail` doctests proving unready pods cannot be routed and zero exit codes cannot build `FailedPod`.
- Reject all illegal combinations inside `PodPhase::from_spec`, un-ignore `lab_goals`, and compare against [solutions/unit-03-representable-valid/src/lib.rs](../../solutions/unit-03-representable-valid/src/lib.rs).

# Commands

```sh
cargo test -p unit-03-representable-valid
cargo test -p unit-03-representable-valid -- --ignored
```
