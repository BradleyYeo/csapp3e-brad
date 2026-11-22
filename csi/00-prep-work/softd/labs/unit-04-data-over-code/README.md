# Unit 4 Lab — The Data Over Code Principle

Study [04-data-over-code-principle.md](../../curriculum/04-data-over-code-principle.md) before inspecting or refactoring [src/lib.rs](src/lib.rs).

# Scenario

An observability engine filters incoming `LogEvent` records using `AlertRule` predicates built from boxed closures (`Box<dyn Fn(&LogEvent) -> bool>`) and stores events in an `EventStore` whose `pub events: Vec<LogEvent>` field is exposed directly to callers. Because closures are opaque behavior and storage is public, rules cannot be serialized, compared, simplified, or used to skip irrelevant service partitions.

# Diagnosis Prompts

## Closures vs. Inspectable Data

- Why are `to_dsl`, `referenced_services`, `simplify`, and `structural_eq` impossible to implement cleanly over `Box<dyn Fn(&LogEvent) -> bool>`?
- Which Parnas "design secret" does `pub events: Vec<LogEvent>` leak to callers?
- How does turning `AlertRule` into a defunctionalized `FilterExpr` AST enable both DSL serialization and query index acceleration inside `EventIndex`?

# Refactoring Tasks

- Replace `Box<dyn Fn(&LogEvent) -> bool>` with a defunctionalized `FilterExpr` enum deriving `Debug, Clone, PartialEq, Eq`.
- Implement `eval(&self, event: &LogEvent) -> bool`, `serialize(&self) -> String`, `parse(&str) -> Result<FilterExpr, String>`, `referenced_services(&self) -> Vec<String>`, and `simplify(self) -> FilterExpr`.
- Replace `EventStore` with a Parnas-encapsulated `EventIndex` hiding storage internals privately and maintaining an internal service index (`HashMap<String, Vec<usize>>`) used when querying service-scoped rules.
- Update and un-ignore `lab_goals` tests and compare against [solutions/unit-04-data-over-code/src/lib.rs](../../solutions/unit-04-data-over-code/src/lib.rs).

# Commands

```sh
cargo test -p unit-04-data-over-code
cargo test -p unit-04-data-over-code -- --ignored
```
