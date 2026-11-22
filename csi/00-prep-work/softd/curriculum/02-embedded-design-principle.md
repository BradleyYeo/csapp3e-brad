# The Embedded Design Principle

Good architecture is not a diagram stored separately from the codebase. You should be able to derive the code's structure mechanically from the design so that the design remains visibly embedded in the code.

# Deriving Code From Design

## Mechanical Correspondence

- Each independent domain concept should map to a distinct, named boundary in code rather than dissolving into ad-hoc maps and helper flags.
- When structure mirrors design, changing a domain rule points directly to one type or function instead of requiring full-codebase grep.

## Signs Design Is Missing From Code

- Flat `HashMap<String, String>` bags carrying unrelated fields across boundaries.
- Compound strings with custom separators (`"env:prod|region:us-east"`) parsed repeatedly inside unrelated functions.
- Dispatch ladders scattered across multiple functions that must stay synchronized whenever a new step or stage is added.

# Coupling Taxonomy

## Forms of Hidden Coupling

- Format coupling (stringly typed): Producers and consumers implicitly share delimiter rules, key names, and encoding assumptions.
- Shared-secret coupling: Multiple components parse or interpret the same raw payload instead of consuming a parsed domain value.
- Temporal coupling: Calling function `B` silently assumes function `A` mutated a field or map key first.
- Control coupling: Passing flags or magic string prefixes (`"hotfix-"`) to alter another module's internal control flow.
- Implicit-assumption coupling: Code assumes values lack special characters (`|`, `:`, `/`) without enforcing it at construction.

# Couriers vs. Prying Components

## The Courier Anti-Pattern

- A courier's job is to deliver a package from source to destination (e.g., routing an artifact, forwarding an event, queuing a job).
- A **prying courier** opens the opaque payload to inspect fields or magic prefixes (`image.starts_with("hotfix-")`) and silently alters routing behavior.
- Prying entangles transport concerns with payload representation, violating Daniel Jackson's concept independence.

## Enforcing Opacity via Parametricity

- In Rust, replacing concrete payload inspection with a generic type parameter `T` (`fn relay<T>(&mut self, envelope: RoutingEnvelope<T>)`) enforces parametricity at compile time.
- Because `ArtifactRelay` knows nothing about `T` (unless constrained by explicit trait bounds), it is physically impossible for the courier to pry into payload fields.
- Any routing decision must be explicit in `RoutingEnvelope`'s own header fields, keeping intent in a single place.

# Mental Model and Connections

- Jackson connection: Keep concepts independent (`DeployTarget` vs. `ReleaseArtifact` vs. `ArtifactRelay`) and link them explicitly through clean synchronizations or envelopes, never through hidden backdoors.
- Unit 2 lab preview: In [unit-02-embedded-design](../labs/unit-02-embedded-design/README.md), you eliminate stringly-typed `HashMap` pipelines, extract a typed stage pipeline, and seal `ArtifactRelay` with parametric generics.

# Active Recall

- What makes a courier "prying," and why does Rust parametricity (`fn send<T>(env: Envelope<T>)`) prevent it statically?
- Name three distinct forms of coupling introduced by `"env:prod|region:us-east"` stored inside a `HashMap<String, String>`.
- How does the Embedded Design Principle help a newcomer recover programmer intent without reading external docs?
