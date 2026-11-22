# The Data Over Code Principle

Prefer representing domain logic, rules, and workflows as inspectable data structures interpreted by small engines rather than burying behavior inside opaque code and closures.

# Parnas Information Hiding

## Decomposing by Secrets, Not Flowcharts

- David Parnas showed that decomposing a system by execution steps ("step 1 reads input, step 2 processes, step 3 writes") couples every module to shared data formats.
- Decompose instead by **design secrets**—decisions likely to change independently (storage layout, indexing strategy, wire format, caching policy).
- Each module exposes only the minimal interface callers need and hides its secret behind private representations. If a caller reaches into `pub events: Vec<LogEvent>`, the storage secret is leaked and cannot evolve from a linear scan to an index.

# Defunctionalization

## Turning Closures Into Data

- Higher-order functions (`Box<dyn Fn(&LogEvent) -> bool>`) are maximally flexible at runtime, yet completely opaque to inspection.
- You cannot serialize a closure to JSON or a DSL, check two closures for equality (`PartialEq`), extract referenced service names for query planning, or optimize `Not(Not(x))` away.
- **Defunctionalization** replaces each function/combinator with a data constructor in an enum AST (`FilterExpr`) and moves execution into one central interpreter (`eval(&FilterExpr, &LogEvent) -> bool`).

## Payoffs of Data Over Code

- Serialization and wire transport: Rules can be saved, transmitted, and parsed cleanly.
- Structural equality and diffing: Tests and caches can compare rule ASTs directly.
- Static analysis and query optimization: An index can inspect `FilterExpr` to extract `referenced_services` and avoid scanning unrelated partitions.
- Algebraic simplification: Redundant predicates (`And(Always, x)` -> `x`, `Not(Not(x))` -> `x`) collapse before execution.

# Mental Model and Connections

- Coupling Data Over Code with Parnas Information Hiding creates a clean contract: callers build rich, transparent `FilterExpr` data while `EventIndex` hides how it indexes and evaluates those rules internally.
- Unit 4 lab preview: In [unit-04-data-over-code](../labs/unit-04-data-over-code/README.md), you defunctionalize closure-based `AlertRule` filters into a round-trip DSL AST (`FilterExpr`) and seal `EventStore` behind a Parnas-encapsulated `EventIndex`.

# Active Recall

- Why is decomposing a codebase by flowchart steps usually worse for evolvability than decomposing by Parnas design secrets?
- List four capabilities you gain when replacing `Box<dyn Fn(&T) -> bool>` with an explicit AST enum and `eval` interpreter.
- What is the main tradeoff of defunctionalization (hint: consider the expression problem and openness to arbitrary caller predicates)?
