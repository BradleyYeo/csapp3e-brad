# Future-Proofing Code

Code rots when invisible assumptions about "the World That Was" leak across module boundaries and lock the architecture into complexity ratchets. Future-proofing is the discipline of keeping change cheap.

# The RAD Process

## Reduce and Sequester Assumptions

- Every line of code embeds assumptions about the surrounding world (e.g., keys never contain `/`, timestamps only move forward, stores never evict).
- **Reduce** assumptions by weakening preconditions and removing unnecessary constraints.
- **Sequester** unavoidable assumptions behind one codec, constructor, or boundary module so changing a rule (such as key delimiter encoding) edits one place.

## Add Openness

- Replace closed `enum BackendKind` dispatch switches with trait abstractions or extensible data pipelines so callers can supply new implementations without modifying library code.

## Diminish Complexity Ratchets

- A **complexity ratchet** occurs when adding a quick special case or leaky subtype forces every future caller to add defensive workarounds, making cleanup progressively harder.
- Stop ratchets by fixing broken contracts at the interface boundary rather than patching callers around them.

# Behavioral Subtyping and Liskov Substitution (LSP)

## Substitutability Rules

- Subtype `S` is a true behavioral subtype of `T` only if code written against `T`'s specification remains correct for every `S`.
- **Preconditions may only be weakened** (accept at least everything `T` accepts).
- **Postconditions may only be strengthened** (guarantee at least everything `T` guarantees).
- **Invariants must be preserved** across all methods.

## Classic LSP Traps

- Implementing a full `KvStore` trait on a `ReadOnlyReplica` that returns `Err("read-only")` on `put` strengthens preconditions (or adds an unexpected failure mode), breaking generic callers expecting writable stores.
- Implementing `KvStore` on a `BoundedStore` that silently evicts old keys on `put` weakens the durability postcondition (`after put(k, v), get(k) == Some(v)` for earlier keys).

# Parnas Subsets and Program Families

## Designing Minimal Capability Hierarchies

- Instead of forcing every backend into one fat trait (`KvStore`) and stubbing unsupported operations, decompose capabilities into **Parnas subsets** ordered by what a caller strictly needs.
- Example hierarchy: `KvReader` (read-only queries) -> `KvStore: KvReader` (durable writes/deletes), with `EvictingCache: KvReader` placed in a separate sibling branch with honest cache-eviction semantics.
- A read-only inspector depends only on `KvReader`, making it automatically usable on durable stores, read-only snapshots, and evicting caches alike.

# Mental Model and Connections

- Ousterhout connection: Complexity ratchets are the formal mechanism behind tactical programming death spirals; RAD breaks the ratchet before it locks.
- Unit 6 lab preview: In [unit-06-future-proofing](../labs/unit-06-future-proofing/README.md), you split a fat `KvStore` trait with LSP-violating replicas/caches into honest Parnas subsets, sequester key-encoding assumptions, and generalize `CheckpointManager` for arbitrary user backends.

# Active Recall

- Explain why `ReadOnlyReplica` implementing `KvStore` by returning `Err` on `put` violates LSP even though the types match.
- Why does a `BoundedStore` that silently drops older keys violate `KvStore`'s Hoare postcondition?
- Describe the three steps of the RAD process and give a concrete example of sequestering an assumption in Rust.
- How do Parnas subsets differ from simply throwing `unimplemented!()` or `Err("unsupported")` inside a fat trait?
