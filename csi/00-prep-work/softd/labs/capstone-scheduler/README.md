# Capstone Lab — Mini Container Workload Scheduler

Study [07-capstone-guide.md](../../curriculum/07-capstone-guide.md) before inspecting or refactoring [src/lib.rs](src/lib.rs).

# Scenario

This mini container workload scheduler integrates six distinct architectural flaws corresponding directly to Units 1 through 6:

- Unit 1 (Hidden Logic): `NodeResources::reserve` deducts CPU before checking memory, leaking reserved CPU whenever memory is exhausted.
- Unit 2 (Embedded Design): Node labels are encoded as `"k=v;k2=v2"` strings and `AuditRelay` pries into workload names starting with `"canary-"` to mute audit events.
- Unit 3 (Representable/Valid): `Workload` mixes nullable `assigned_node`, `status: String`, and `allocated_cpu`, allowing bound workloads without nodes and pending workloads with allocations.
- Unit 4 (Data Over Code): `PlacementConstraint` wraps an opaque `Box<dyn Fn(&Node) -> bool>`, blocking DSL serialization, rule inspection, and clear rejection explanations.
- Unit 5 (Algebraic Refactoring): `SchedulingPolicy` combines unfactored booleans (`require_gpu`, `allow_spot`, `preemptible`) with duplicated scoring arms where `allow_spot` silently skips GPU enforcement.
- Unit 6 (Future-Proofing): `ClusterBackend` forces `DryRunBackend` to implement `bind_workload` by silently dropping writes (an LSP violation) and blocks custom backends.

# Refactoring Tasks

- Fix `NodeResources::reserve` to be atomic and document Hoare invariants (`used <= capacity`).
- Replace string-encoded labels with structured `BTreeMap<String, String>` and parameterize `AuditRelay<E>` generically so it cannot pry into event names.
- Split `Workload` into `PendingWorkload` and `BoundWorkload` (inside `WorkloadState`) with smart constructors for `WorkloadName` and positive `ResourceRequest`.
- Replace `PlacementConstraint` closures with a defunctionalized `PlacementRule` AST supporting `eval`, `explain_failure`, `serialize`, and `parse`.
- Factor `SchedulingPolicy` into an exact algebraic `WorkloadTier` + `AcceleratorNeed` model with a single shared GPU check.
- Split `ClusterBackend` into `ClusterView` (read-only) and `ClusterMutator: ClusterView` (mutation), removing the LSP-violating `DryRunBackend` write lie in favor of an honest `ClusterSnapshot` / `DryRunSimulator` implementing `ClusterView`.
- Un-ignore all six `lab_goals` tests and compare against [solutions/capstone-scheduler/src/lib.rs](../../solutions/capstone-scheduler/src/lib.rs).

# Commands

```sh
cargo test -p capstone-scheduler
cargo test -p capstone-scheduler -- --ignored
```
