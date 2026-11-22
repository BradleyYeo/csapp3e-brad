# Capstone Integration Guide

The capstone brings all six units together inside a single realistic systems domain: a mini container workload scheduler and policy engine ([capstone-scheduler](../labs/capstone-scheduler/README.md)).

# Unit-to-Capstone Mapping

## Unit 1 — Hidden Logic and Atomic Reservations

- `NodeResources::reserve` and `release` must preserve capacity invariants (`used <= capacity`) under all inputs and perform multi-resource (`cpu` + `memory`) checks atomically so a failed memory check never leaks reserved CPU.

## Unit 2 — Embedded Design and Opaque Couriers

- Replace `"k=v;k2=v2"` label strings with typed label maps or collections so arbitrary label values cannot corrupt parsing.
- Parameterize `AuditRelay<E>` generically so the courier forwards scheduling events without prying into workload names (such as muting `"canary-"` workloads).

## Unit 3 — Representable/Valid Workload Lifecycle

- Replace a flat `Workload` struct containing nullable `assigned_node`, `allocated_cpu`, and stringly `status` with distinct states (`PendingWorkload`, `BoundWorkload`, or `WorkloadState` variants) so unbound workloads cannot carry node allocations and bound workloads always identify their node.

## Unit 4 — Data Over Code Placement Rules

- Replace opaque `Box<dyn Fn(&Node) -> bool>` placement constraints with a defunctionalized `PlacementRule` AST supporting evaluation, serialization/parsing, equality comparison, and human-readable failure explanations.

## Unit 5 — Algebraic Refactoring of Scheduling Policy

- Replace unfactored policy booleans (`require_gpu`, `allow_spot`, `preemptible`) and duplicated scoring ladders with a tight algebraic `SchedulingClass` / `AffinityPolicy` factored around shared cost evaluation.

## Unit 6 — Future-Proofing With Parnas Subsets and RAD

- Split `ClusterBackend` into `ClusterView` (read-only inspection) and `ClusterMutator: ClusterView` (binding/mutation) so `DryRunSimulator` can honestly simulate read queries or isolated snapshots without silently dropping `bind` mutations behind an LSP violation.
- Sequester cluster key/namespace formatting behind one boundary rather than scattering separator assumptions.

# Review Rubric Against the 11 Aspects

- Done: All six unit-mapped capstone tests and `compile_fail` doctests pass with zero warnings.
- Understandable and Modularly Understandable: Every public type states invariants; `NodeResources` and `PlacementRule` can be verified locally.
- Intent in a Single Place: Scheduling eligibility, resource math, and event routing each have one canonical definition.
- Robust and Hides Secrets: Failed binds never corrupt node capacity; cluster storage and key encoding stay private.
- Isolates Assumptions and Open: Arbitrary label values work safely, and new `ClusterView` / `ClusterMutator` implementations plug in cleanly.

# Post-Refactor Reflection

- Which planted flaw in the starter capstone caused the hardest-to-spot infection when reading only happy-path tests?
- Where did making illegal states unrepresentable eliminate defensive error checks from the scheduler loop?
- Which boundary in your capstone could change tomorrow (new rule operator, new storage backend, new node resource dimension) with minimal ripple?
