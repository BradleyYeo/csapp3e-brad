# Unit 6 Lab — Future-Proofing Code

Study [06-future-proofing-code.md](../../curriculum/06-future-proofing-code.md) before inspecting or refactoring [src/lib.rs](src/lib.rs).

# Scenario

A distributed checkpoint manager stores state snapshots behind a fat `KvStore` trait. Three backends implement `KvStore`: `InMemoryStore`, `ReadOnlyReplica` (which returns `Err` on `put` and `delete`), and `BoundedStore` (which silently evicts older keys when full). Meanwhile, `CheckpointManager` hardcodes `BackendKind`, scatters raw `'/'` assumptions across multiple methods, and cannot accept custom storage backends.

# Diagnosis Prompts

## RAD, LSP, and Parnas Subsets

- Which Hoare postcondition of `KvStore` does `BoundedStore` violate when capacity fills up?
- Why does `ReadOnlyReplica` returning `Err("read-only")` on `put` violate Liskov Substitution even though the Rust type signatures match `KvStore`?
- Where does the `"World That Was"` assumption (`namespace` and `checkpoint` contain no `'/'`) leak across `CheckpointManager`?
- How do Parnas subset traits (`KvReader`, `KvStore: KvReader`, `EvictingCache: KvReader`) eliminate fake implementations while keeping read-only inspection reusable across all backends?

# Refactoring Tasks

- Split `KvStore` into `KvReader` (read-only queries) and `KvStore: KvReader` (durable writes/deletes), plus a separate `EvictingCache: KvReader` trait for bounded caches.
- Move `ReadOnlySnapshot` onto `KvReader` only and `BoundedCache` onto `KvReader + EvictingCache` only so neither pretends to satisfy `KvStore` durability.
- Sequester namespace/checkpoint encoding behind one private `CheckpointKey` codec (e.g., percent-escaping `'%'` and `'/'`) so arbitrary characters including `'/'` and `".."` round-trip without collisions.
- Parameterize `CheckpointManager<S: KvStore>` (and `CheckpointInspector<R: KvReader>`) generically instead of hardcoding `BackendKind`.
- Add `compile_fail` doctests proving `ReadOnlySnapshot` and `BoundedCache` cannot satisfy `CheckpointManager<S: KvStore>`.
- Update and un-ignore `lab_goals` tests and compare against [solutions/unit-06-future-proofing/src/lib.rs](../../solutions/unit-06-future-proofing/src/lib.rs).

# Commands

```sh
cargo test -p unit-06-future-proofing
cargo test -p unit-06-future-proofing -- --ignored
```
